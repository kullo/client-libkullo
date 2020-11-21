/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/util/crash.h"

#include <stdexcept>
#include <iostream>

namespace Kullo {
namespace Util {

void Crash::nullptrDereference()
{
    int *null = nullptr;
    std::cout << *null;
}


K_NORETURN void Crash::throwException()
{
    throw std::runtime_error("error to test breakpad");
}

}
}
