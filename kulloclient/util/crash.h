/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Util {

class Crash
{
public:
    // -> SIGSEGV
    static void nullptrDereference();
    // -> std::terminate()
    K_NORETURN static void throwException();
};

}
}
