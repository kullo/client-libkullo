/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <map>
#include <thread>

#ifdef _MSC_VER
#define LL_FUNCTION_INFO  __FUNCSIG__
#else
#define LL_FUNCTION_INFO  __PRETTY_FUNCTION__
#endif

#ifndef Log
#define Log Kullo::Util::LibraryLogger(__FILE__, __LINE__, LL_FUNCTION_INFO)
#endif

namespace Kullo {
namespace Util {

class ThreadName : public std::string
{
public:
    ThreadName(): std::string("?") {}
    ThreadName(const std::string &str): std::string(str) {}
};

class LibraryLogger final
{
public:
    enum struct LogType {
        None = 0,
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };
    using LogFunctionType = std::function<void(
        const std::string &file,
        const int line,
        const std::string &function,
        const LogType type,
        const std::string &msg,
        const std::string &thread,
        const std::string &stacktrace
    )>;

    LibraryLogger(const char *file, int line, const char *function);
    ~LibraryLogger();

    LibraryLogger &f();
    LibraryLogger &e();
    LibraryLogger &w();
    LibraryLogger &i();
    LibraryLogger &d();

    LibraryLogger &f(const std::string &stacktrace);
    LibraryLogger &e(const std::string &stacktrace);
    LibraryLogger &w(const std::string &stacktrace);
    LibraryLogger &i(const std::string &stacktrace);
    LibraryLogger &d(const std::string &stacktrace);

    template <typename T>
    LibraryLogger &operator<<(T input) {
        write(input);
        return *this;
    }

    static void setLogFunction(LogFunctionType logFunction);
    static void setCurrentThreadName(const std::string &name);

    static void defaultLogWrapper(
            const std::string &file,
            const int line,
            const std::string &function,
            const LogType type,
            const std::string &msg,
            const std::string &thread,
            const std::string &stacktrace);

private:
    // strings: add quotes
    void write(const std::string &input);

    // booleans: convert to true/false
    void write(bool input);

    // all other types: use operator<<
    template <typename T>
    void write(T input)
    {
        buffer_ << input;
    }

    void setLogType(LogType type);
    void setStacktrace(const std::string &stacktrace);

    static LogFunctionType logfunction_;
    static std::map<std::thread::id, ThreadName> threadNames_;

    std::string file_;
    int line_;
    std::string function_;
    std::string stacktrace_;
    LogType type_ = LogType::None;
    std::ostringstream buffer_;
};

}
}
