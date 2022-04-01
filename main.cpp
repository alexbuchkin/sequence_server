#include <arpa/inet.h>
#include <cstring>
#include <future>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>


#include "client_handler.h"

int main()
{
	int listenerFd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenerFd == -1) {
		perror("Unable to create socket");
		return 1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(12345);

	if (bind(listenerFd, (const sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("Failed to bind");
		return 1;
	}

	std::cerr << "Start listening\n";
	if (listen(listenerFd, 1) == -1) {
		perror("Failed to listen");
		return 1;
	}

	int new_socketfd = accept(listenerFd, nullptr, nullptr);
	std::cerr << "accepted!\n";
	
	std::future<void> future = std::async(std::launch::async, [new_socketfd](){
		ClientHandler(new_socketfd).Handle();
	});
	future.wait();

	close(listenerFd);
	std::cerr << "All done!\n";
	return 0;
}
