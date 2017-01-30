/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
