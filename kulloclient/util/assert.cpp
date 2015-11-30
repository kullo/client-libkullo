/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/assert.h"

#include "kulloclient/util/stacktrace.h"

namespace Kullo {
namespace Util {

void assertionFailed(const std::string &file,
                          int line,
                          const std::string &func,
                          const std::string &expr)
{
    auto stacktrace = Stacktrace::toString(1);
    throw AssertionFailed(file, line, func, expr, stacktrace);
}

AssertionFailed::AssertionFailed(const std::string &file,
                                 int line,
                                 const std::string &func,
                                 const std::string &expr,
                                 const std::string &stacktrace) throw()
    : file_(file),
      line_(line),
      function_(func),
      expression_(expr),
      stacktrace_(stacktrace)
{
    message_ = "Assertion failed in "
                + file_ + ":" + std::to_string(line_)
                + "\n\tFunction: " + function_ + "\n\tExpression: " + expression_
            + "\n" + stacktrace_;
}

const char *AssertionFailed::what() const throw()
{
    return message_.c_str();
}

}
}
