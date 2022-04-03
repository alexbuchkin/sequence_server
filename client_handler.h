#pragma once

#include <array>
#include <cctype>
#include <cstring>
#include <errno.h>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "error_message.h"
#include "sequence.h"

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
    ClientHandler(int socketFd);
    ~ClientHandler();
    void Handle();
    
private:
    void HandleCommand();
    void HandleUpdateSequenceCommand(const CommandInfo& commandInfo);
    void HandleExportSequencesCommand();
    void SendMessage(const std::string& message);

public:
    static const size_t NUMBER_OF_SEQUENCES = 3;
    
public:
    static const size_t BUFFER_LENGTH = 100;
    
private:
    bool IsHandling = true;
    const int SocketFd;
    std::array<Sequence, NUMBER_OF_SEQUENCES> Sequences;
    char Buffer[BUFFER_LENGTH];
};
