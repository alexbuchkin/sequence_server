#include "connection_holder.h"

ConnectionHolder::ConnectionHolder()
{
    InterruptedConnectionsCleaner = std::async(std::launch::async, [this](){
        std::queue<uint64_t> interruptedConnectionIds;

        while (!IsSignalCaught) {
            if (!Pipe_.Queue.empty()) {
                {
                    std::lock_guard lock(Pipe_.Mutex);
                    while (!Pipe_.Queue.empty()) {
                       interruptedConnectionIds.push(Pipe_.Queue.front());
                       Pipe_.Queue.pop();
                    }
                }

                {
                    std::lock_guard lock(Mutex);
                    while (!interruptedConnectionIds.empty()) {
                        uint64_t id = interruptedConnectionIds.front();
                        interruptedConnectionIds.pop();
                        auto it = Storage.find(id);
                        it->second.wait();
                        Storage.erase(it);
                    }
                }
            }

            sleep(1);
        }
    });
}

ConnectionHolder::~ConnectionHolder()
{
    InterruptedConnectionsCleaner.wait();
    std::lock_guard lock(Mutex);
    for (const auto& [id, future] : Storage) {
        future.wait();
    }
}

void ConnectionHolder::Add(int socketFd)
{
    std::lock_guard lock(Mutex);
    Storage[Counter] = std::async(std::launch::async, [this, connectionId = Counter, socketFd](){
        ClientHandler(connectionId, socketFd, Pipe_).Handle();
    });
    Counter++;
}

