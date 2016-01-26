/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>

namespace Kullo {
namespace Util {

/// Utility class for creating a stack trace
class Stacktrace
{
public:
    /// Returns a stack trace as a string
    static std::string toString(size_t skip = 0);

    /// Prints a stack trace to standard error
    static void toStdErr();
};

}
}
