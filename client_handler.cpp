#include "client_handler.h"

namespace {

/*
 * Trims heading and trailing space characters
 */
std::string Trim(const std::string& str)
{
    const size_t firstNonSpaceIndex = std::distance(str.cbegin(),
        std::find_if(str.cbegin(), str.cend(), [](char c){ return !isspace(c); }));
    const size_t lastNonSpaceIndex = str.length() - std::distance(str.crbegin(),
        std::find_if(str.crbegin(), str.crend(), [](char c){ return !isspace(c); })) - 1;
        
    if (firstNonSpaceIndex < lastNonSpaceIndex) {
        return str.substr(firstNonSpaceIndex,
            lastNonSpaceIndex - firstNonSpaceIndex + 1);
    }
    
    return "";
}

CommandInfo ParseCommand(const std::string& receivedText)
{
    const std::string trimmedText = Trim(receivedText);
    CommandInfo commandInfo;
    
    if (trimmedText == "export seq") {
        commandInfo.Type = CommandType::EXPORT_SEQUENCE;
        return commandInfo;
    }
    
    static const std::regex regex("^seq([0-9]+) ([0-9]+) ([0-9]+)$");
    std::smatch match;
    if (!std::regex_match(trimmedText, match, regex)) {
        commandInfo.Type = CommandType::ERROR;
        commandInfo.ErrorInfo = std::string("Unable to parse command \"") + 
            trimmedText + "\"";
        return commandInfo;
    }
    
    if (!(std::istringstream(match[1].str()) >> commandInfo.SeqNumber)
        || commandInfo.SeqNumber == 0
        || commandInfo.SeqNumber > ClientHandler::NUMBER_OF_SEQUENCES) {
        std::ostringstream out;
        out << "Invalid sequence number \"" << match[1].str()
            << "\". Must be in range [1, "
            << ClientHandler::NUMBER_OF_SEQUENCES << "]";
        commandInfo.ErrorInfo = out.str();
    } else if (!(std::istringstream(match[2].str()) >> commandInfo.Base)) {
        std::ostringstream out;
        out << "Invalid base argument \"" << match[2].str()
            << "\". Must fit in uint64_t";
        commandInfo.ErrorInfo = out.str();
    } else if (!(std::istringstream(match[3].str()) >> commandInfo.Step)) {
        std::ostringstream out;
        out << "Invalid step argument \"" << match[3].str()
            << "\". Must fit in uint64_t";
        commandInfo.ErrorInfo = out.str();
    }
    
    if (!commandInfo.ErrorInfo.empty()) {
        commandInfo.Type = CommandType::ERROR;
        return commandInfo;
    }
    
    commandInfo.Type = CommandType::UPDATE_SEQUENCE;
    commandInfo.SeqNumber--;
    return commandInfo;
}
    
} // namespace

ClientHandler::ClientHandler(uint64_t connectionId, int socketFd, Pipe& pipe)
    : ConnectionId(connectionId)
    , SocketFd(socketFd)
    , Pipe_(pipe)
    {}

ClientHandler::~ClientHandler()
{
    if (close(SocketFd) == -1) {
        PRINT_PERROR_MESSAGE("Unable to close socket");
    }

    std::lock_guard(Pipe_.Mutex);
    Pipe_.Queue.push(ConnectionId);
}

void ClientHandler::Handle()
{
    while (!IsSignalCaught && IsHandling) {
        memset(Buffer, 0, sizeof(Buffer));
        ssize_t bytesRead = recv(SocketFd, Buffer, sizeof(Buffer), MSG_DONTWAIT);
        if (bytesRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            PRINT_PERROR_MESSAGE("Cannot read anything from client");
            IsHandling = false;
            break;
        } else if (bytesRead > 0) {
            HandleCommand();
        }
    }
}

void ClientHandler::HandleCommand()
{
    auto commandInfo = ParseCommand(std::string(Buffer));
    if (commandInfo.Type == CommandType::ERROR) {
        SendMessage(commandInfo.ErrorInfo);
        return;
    } else if (commandInfo.Type == CommandType::UPDATE_SEQUENCE) {
        HandleUpdateSequenceCommand(commandInfo);
    } else {
        HandleExportSequencesCommand();
    }
}

void ClientHandler::HandleUpdateSequenceCommand(const CommandInfo& commandInfo)
{
    if (Sequences[commandInfo.SeqNumber].IsSet()) {
        std::ostringstream out;
        out << "Sequence " << commandInfo.SeqNumber + 1 << " is already set";
        SendMessage(out.str());
        return;
    }
    
    if (commandInfo.Base != 0 && commandInfo.Step != 0) {
        Sequences[commandInfo.SeqNumber].Set(commandInfo.Base, commandInfo.Step);
    }
}

void ClientHandler::HandleExportSequencesCommand()
{
    while (!IsSignalCaught && IsHandling) {
        std::ostringstream out;
        bool isEmpty = true;

        for (auto& seq : Sequences) {
            if (!seq.IsSet()) {
                continue;
            }

            if (!isEmpty) {
                out << " ";
            }
            out << seq.GetNextValue();
            isEmpty = false;
        }
        
        if (!isEmpty) {
            SendMessage(out.str());
        }
        sleep(1);
    }
}

void ClientHandler::SendMessage(const std::string& message)
{
    std::string toSend = message + "\n";
    size_t totalSent = 0;
    while (totalSent != toSend.length()) {
        ssize_t bytesSent = send(SocketFd, toSend.c_str() + totalSent,
            toSend.length() - totalSent, MSG_NOSIGNAL);
        if (bytesSent <= 0) {
            std::ostringstream out;
            out << "Failed to send message \"" << message << "\"";
            PRINT_PERROR_MESSAGE(out.str());
            IsHandling = false;
            break;
        }
        
        totalSent += bytesSent;
    }
}

