#include "server.h"

using namespace std::chrono_literals;

Server::Server(uint16_t port)
	: Port(port)
{
	LOG("Constructing server");
	ListeningSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (ListeningSocketFd == -1) {
		PRINT_PERROR_MESSAGE("Failed to open listening socket");
		exit(1);
	}
	LOG(std::string("Listening socket fd is ") + std::to_string(ListeningSocketFd));
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(Port);
	
	if (bind(ListeningSocketFd, (const sockaddr*)&addr, sizeof(addr)) == -1) {
		PRINT_PERROR_MESSAGE("Failed to bind listening socket");
		exit(1);
	}
	
	if (listen(ListeningSocketFd, 1) == -1) {
		PRINT_PERROR_MESSAGE("Failed to listen");
		exit(1);
	}
}

Server::~Server()
{
	LOG("Destructing server");
	for (auto& future : Connections) {
		future.wait();
	}

	if (close(ListeningSocketFd) == -1) {
		PRINT_PERROR_MESSAGE("Failed to close listening socket");
	}
}

void Server::Run()
{
	while (true) {
		int newSocketFd = accept(ListeningSocketFd, nullptr, nullptr);
		if (newSocketFd == -1) {
			PRINT_PERROR_MESSAGE("Failed to accept");
			continue;
		}
		
		if (Connections.size() >= MAX_CONNECTIONS) {
			RemoveInterruptedConnections();
		}
		if (Connections.size() >= MAX_CONNECTIONS) {
			PRINT_ERROR_MESSAGE("Too many opened connections");
			continue;
		}
		
		Connections.push_back(std::async(std::launch::async, [newSocketFd](){
			ClientHandler(newSocketFd).Handle();
		}));
	}
}

void Server::RemoveInterruptedConnections()
{
	LOG("Removing interrupted connections");
	for (auto it = Connections.begin(); it != Connections.end();) {
		if (it->wait_for(0ms) == std::future_status::ready) {
			LOG("Removing one");
			it = Connections.erase(it);
		} else {
			++it;
		}
	}
}
