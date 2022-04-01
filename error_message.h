#pragma once

#include <errno.h>
#include <iostream>
#include <sstream>
#include <cstdio>

#define PRINT_ERROR_MESSAGE(msg) \
do {  \
	std::cerr << __FILE__ << ":" << __LINE__ << "   " << (msg) << std::endl;  \
} while(0);

#define PRINT_PERROR_MESSAGE(msg) \
do {  \
	std::ostringstream out;  \
	out << __FILE__ << ":" << __LINE__ << "   " << (msg) << std::endl;  \
	perror(out.str().c_str());  \
} while(0);
