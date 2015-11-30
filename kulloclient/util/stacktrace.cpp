/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/stacktrace.h"

#if (!defined(_WIN32) && !defined(__ANDROID__))
    #define KULLO_EXECINFO_AVAILABLE
#endif

#ifdef KULLO_EXECINFO_AVAILABLE
    #include <execinfo.h>
    #include <stdlib.h>
    #include <unistd.h>
#endif


namespace Kullo {
namespace Util {

std::string Stacktrace::toString(size_t skip)
{
    std::string result;

#ifdef KULLO_EXECINFO_AVAILABLE
    void *array[20];
    size_t size = backtrace(array, 20);
    char **btSymbols = backtrace_symbols(array, size);
    if (!btSymbols) return std::string();

    bool first = true;
    for (size_t i = 1 + skip; i < size; ++i)
    {
        if (first) first = false; else result += "\n";
        result += btSymbols[i];
    }
    free(btSymbols);
#else
    static_cast<void>(skip); // silence unused warning
#endif

    return result;
}

void Stacktrace::toStdErr()
{
#ifdef KULLO_EXECINFO_AVAILABLE
    void *array[20];
    size_t size;

    size = backtrace(array, 20);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
}

}
}
