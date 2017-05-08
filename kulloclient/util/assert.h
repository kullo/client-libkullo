/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <exception>
#include <string>

#include "kulloclient/util/misc.h"

#ifdef _MSC_VER
#define KULLO_PRETTY_FUNCTION __FUNCSIG__
#else
#define KULLO_PRETTY_FUNCTION __PRETTY_FUNCTION__
#endif

#define kulloAssert(expr) \
    ((expr) \
    ? (void)(0) \
    : ::Kullo::Util::assertionFailed(__FILE__, __LINE__, KULLO_PRETTY_FUNCTION, #expr))
#define kulloAssertMsg(expr, msg) \
    ((expr) \
    ? (void)(0) \
    : ::Kullo::Util::assertionFailed(__FILE__, __LINE__, KULLO_PRETTY_FUNCTION, #expr "; " msg))
#define kulloAssertionFailed(msg) \
    ::Kullo::Util::assertionFailed(__FILE__, __LINE__, KULLO_PRETTY_FUNCTION, msg)

namespace Kullo {
namespace Util {

class AssertionFailed : public std::exception
{
public:
    AssertionFailed(
            const std::string &file,
            int line,
            const std::string &func,
            const std::string &expr,
            const std::string &stacktrace) noexcept;

    virtual char const *what() const noexcept;

private:
    std::string file_;
    int line_;
    std::string function_;
    std::string expression_;
    std::string message_;
    std::string stacktrace_;
};

K_NORETURN void assertionFailed(
        const std::string &file,
        int line,
        const std::string &func,
        const std::string &expr);

}
}
