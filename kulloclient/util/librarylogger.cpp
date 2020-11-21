/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "librarylogger.h"

#include <cassert>
#include <iostream>

#if defined(__unix__)
    #include <unistd.h>
    #if defined(_POSIX_THREADS)
        #include <pthread.h>
    #endif
#endif

namespace Kullo {
namespace Util {

LibraryLogger::LogFunctionType LibraryLogger::logfunction_ = &LibraryLogger::defaultLogWrapper;
std::map<std::thread::id, ThreadName> LibraryLogger::threadNames_;

LibraryLogger::LibraryLogger(const char *file, int line, const char *function)
    : file_(file)
    , line_(line)
    , function_(function)
{
}

LibraryLogger::~LibraryLogger()
{
    auto str = buffer_.str();
    if (!str.empty())
    {
        logfunction_(file_,
                     line_,
                     function_,
                     type_,
                     str,
                     threadNames_[std::this_thread::get_id()],
                     stacktrace_);
    }
}

LibraryLogger &LibraryLogger::f() { setLogType(LogType::Fatal);   return *this; }
LibraryLogger &LibraryLogger::e() { setLogType(LogType::Error);   return *this; }
LibraryLogger &LibraryLogger::w() { setLogType(LogType::Warning); return *this; }
LibraryLogger &LibraryLogger::i() { setLogType(LogType::Info);    return *this; }
LibraryLogger &LibraryLogger::d() { setLogType(LogType::Debug);   return *this; }

LibraryLogger &LibraryLogger::f(const std::string &stacktrace) { setLogType(LogType::Fatal);   setStacktrace(stacktrace); return *this; }
LibraryLogger &LibraryLogger::e(const std::string &stacktrace) { setLogType(LogType::Error);   setStacktrace(stacktrace); return *this; }
LibraryLogger &LibraryLogger::w(const std::string &stacktrace) { setLogType(LogType::Warning); setStacktrace(stacktrace); return *this; }
LibraryLogger &LibraryLogger::i(const std::string &stacktrace) { setLogType(LogType::Info);    setStacktrace(stacktrace); return *this; }
LibraryLogger &LibraryLogger::d(const std::string &stacktrace) { setLogType(LogType::Debug);   setStacktrace(stacktrace); return *this; }

void LibraryLogger::setLogFunction(LogFunctionType logFunction)
{
    logfunction_ = logFunction;
}

void LibraryLogger::setCurrentThreadName(const std::string &name)
{
    threadNames_[std::this_thread::get_id()] = name;

#if defined(_POSIX_THREADS)
    // Don't override process name `kullo` or `kullo-debug` in `ps -A`
    if (name != "main")
    {
#if defined(__APPLE__)
        // OS X SDK 10.11's pthread.h only has this signature
        pthread_setname_np(name.data());
#else
        pthread_setname_np(pthread_self(), name.data());
#endif
    }
#endif
}

void LibraryLogger::defaultLogWrapper(
        const std::string &file,
        const int line,
        const std::string &function,
        const LogType type,
        const std::string &msg,
        const std::string &thread,
        const std::string &stacktrace)
{
    (void)file;
    (void)line;
    (void)function;
    (void)stacktrace;

#ifdef NDEBUG
    // Ignore debug logs in release build
    if (type == LogType::Debug) return;
#endif

    std::string logContent;
    switch (type) {
    case LogType::Fatal:   logContent = "[F] " + thread + " " + msg; break;
    case LogType::Error:   logContent = "[E] " + thread + " " + msg; break;
    case LogType::Warning: logContent = "[W] " + thread + " " + msg; break;
    case LogType::Info:    logContent = "[I] " + thread + " " + msg; break;
    case LogType::Debug:   logContent = "[D] " + thread + " " + msg; break;
    default: assert(false);
    }
    std::cerr << logContent << std::endl;

    if (type == LogType::Fatal)
    {
        std::terminate();
    }
}

void LibraryLogger::write(const std::string &input)
{
    buffer_ << "\"" << input << "\"";
}

void LibraryLogger::write(bool input)
{
    if (input) buffer_ << "true";
    else       buffer_ << "false";
}

void LibraryLogger::setLogType(LogType type)
{
    assert(type_ == LogType::None);
    assert(type != LogType::None);
    type_ = type;
}

void LibraryLogger::setStacktrace(const std::string &stacktrace)
{
    stacktrace_ = stacktrace;
}

}
}
