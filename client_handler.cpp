#include "client_handler.h"

namespace {
	
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
	} else if (!(std::istringstream(match[2].str()) >> commandInfo.Base)
		|| commandInfo.Base == 0) {
		std::ostringstream out;
		out << "Invalid base argument \"" << match[2].str()
			<< "\". Must fit in uint64_t and be greater than 0";
		commandInfo.ErrorInfo = out.str();
	} else if (!(std::istringstream(match[3].str()) >> commandInfo.Step)
		|| commandInfo.Step == 0) {
		std::ostringstream out;
		out << "Invalid step argument \"" << match[3].str()
			<< "\". Must fit in uint64_t and be greater than 0";
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

ClientHandler::ClientHandler(int socketFd)
	: SocketFd(socketFd)
{
	LOG(std::string("Constructing ClientHandler #") + std::to_string(SocketFd));
}
	
ClientHandler::~ClientHandler()
{
	LOG(std::string("Destructing ClientHandler #") + std::to_string(SocketFd));
	if (close(SocketFd) == -1) {
		PRINT_PERROR_MESSAGE("Unable to close socket");
	}
}

void ClientHandler::Handle()
{
	while (true) {
		if (!IsHandling) {
			break;
		}

		memset(Buffer, 0, sizeof(Buffer));
		if (recv(SocketFd, Buffer, sizeof(Buffer), 0) <= 0) {
			PRINT_PERROR_MESSAGE(std::string("ClientHandler #") + std::to_string(SocketFd) + "Cannot read anything from client");
			IsHandling = false;
			break;
		}
		
		HandleCommand();
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
		HandleExportSequencesCommand(commandInfo);
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
	
	Sequences[commandInfo.SeqNumber].Set(commandInfo.Base, commandInfo.Step);
}

void ClientHandler::HandleExportSequencesCommand(const CommandInfo& commandInfo)
{
	auto it = std::find_if(Sequences.cbegin(), Sequences.cend(),
		[](const auto& seq){ return !seq.IsSet(); });
	if (it != Sequences.cend()) {
		std::ostringstream out;
		out << "Sequence #" << std::distance(Sequences.cbegin(), it) + 1
			<< " is not set";
		SendMessage(out.str());
		return;
	}
	
	while (IsHandling) {
		std::ostringstream out;
		out << Sequences.begin()->GetNextValue();
		for (auto it = std::next(Sequences.begin()); it != Sequences.end(); ++it) {
			out << " " << it->GetNextValue();
		}
		
		SendMessage(out.str());
		sleep(1);
	}
}

void ClientHandler::SendMessage(const std::string& message)
{
	std::string toSend = message + "\n";
	size_t totalSent = 0;
	while (totalSent != toSend.length()) {
		ssize_t bytesSent = send(SocketFd, toSend.c_str() + totalSent,
			toSend.length() - totalSent, 0);
		if (bytesSent <= 0) {
			std::ostringstream out;
			out << std::string("ClientHandler #") << std::to_string(SocketFd) << "Failed to send message \"" << message << "\"";
			PRINT_PERROR_MESSAGE(out.str());
			IsHandling = false;
			break;
		}
		
		totalSent += bytesSent;
	}
}

