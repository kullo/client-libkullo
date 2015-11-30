/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <exception>
#include <string>

#ifdef _MSC_VER
#define KULLO_PRETTY_FUNCTION __FUNCSIG__
#else
#define KULLO_PRETTY_FUNCTION __PRETTY_FUNCTION__
#endif

#define kulloAssert(expr) \
    ((expr) \
    ? (void)(0) \
    : ::Kullo::Util::assertionFailed(__FILE__, __LINE__, KULLO_PRETTY_FUNCTION, #expr))

namespace Kullo {
namespace Util {

class AssertionFailed : public std::exception
{
public:
    AssertionFailed(const std::string &file,
                    int line,
                    const std::string &func,
                    const std::string &expr,
                    const std::string &stacktrace) throw();
    virtual ~AssertionFailed() = default;

    virtual char const *what() const throw();

private:
    std::string file_;
    int line_;
    std::string function_;
    std::string expression_;
    std::string message_;
    std::string stacktrace_;
};

void assertionFailed(const std::string &file,
                     int line,
                     const std::string &func,
                     const std::string &expr);

}
}
