#pragma once

#include <array>
#include <cctype>
#include <cstring>
#include <errno.h>
#include <iterator>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "error_message.h"
#include "pipe.h"
#include "sequence.h"
#include "signal_handling.h"

enum class CommandType {
    ERROR,
    UPDATE_SEQUENCE,
    EXPORT_SEQUENCE
};

struct CommandInfo {
    CommandType Type;
    size_t SeqNumber;
    uint64_t Base;
    uint64_t Step;
    std::string ErrorInfo;
};

class ClientHandler
{
public:
    ClientHandler(uint64_t connectionId, int socketFd, Pipe& pipe);
    ClientHandler(const ClientHandler&) = delete;

    /*
     * Close SocketFd; push ConnectionId to the Pipe
     * to let ConnectionHolder know that this connection
     * is interrupted so it can be removed from the Storage
     */
    ~ClientHandler();

    /*
     * Read command in eternal cycle
     * then try to execute it
     * via calling HandleCommand
     */
    void Handle();
    
private:
    /*
     * Parse received command
     * and if success then call one of
     * the next two methods,
     * otherwise inform client about error
     */
    void HandleCommand();
    void HandleUpdateSequenceCommand(const CommandInfo& commandInfo);
    void HandleExportSequencesCommand();

    void SendMessage(const std::string& message);

public:
    static const size_t NUMBER_OF_SEQUENCES = 3;
    static const size_t BUFFER_LENGTH = 100;
    
private:
    /*
     * Will be set to false
     * if any recv or send will fail.
     * It will interrupt Handle method
     */
    bool IsHandling = true;
    const uint64_t ConnectionId;
    const int SocketFd;
    std::array<Sequence, NUMBER_OF_SEQUENCES> Sequences;

    /*
     * Holds received command
     */
    char Buffer[BUFFER_LENGTH];

    /*
     * When destructor is called,
     * current ClientHandler must pass
     * its ConnectionId to the Pipe
     */
    Pipe& Pipe_;
};
