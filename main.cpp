#include <cctype>
#include <iostream>
#include <sstream>

#include "server.h"

int main(int argc, char** argv)
{
    uint16_t port = 12345;
    if (argc == 2) {
        std::istringstream in(argv[1]);
        if (!(in >> port)) {
            std::cerr << "Port is invalid. Usage: \"./exec port\"."
                << "Port is 12345 by default\n";
            return 0;
        }
    }
    
    Server(port).Run();
    return 0;
}
