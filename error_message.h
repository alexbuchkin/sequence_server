#pragma once

#include <cstdio>
#include <errno.h>
#include <iostream>
#include <mutex>
#include <sstream>

namespace {
    
std::mutex printMutex;    

} // namespace

/*
 * Print detailed error message
 */
#define PRINT_ERROR_MESSAGE(msg) \
do {  \
    std::ostringstream out;  \
    out << __FILE__ << ":" << __LINE__ << "   " << (msg);  \
    const std::lock_guard lock(printMutex);  \
    std::cerr << out.str() << std::endl;  \
} while(0);

/*
 * Print detailed error message with perror
 */
#define PRINT_PERROR_MESSAGE(msg) \
do {  \
    std::ostringstream out;  \
    out << __FILE__ << ":" << __LINE__ << "   " << (msg);  \
    const std::lock_guard lock(printMutex);  \
    perror(out.str().c_str());  \
} while(0);
