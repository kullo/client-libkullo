/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
