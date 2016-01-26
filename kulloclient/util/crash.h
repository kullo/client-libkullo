/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

namespace Kullo {
namespace Util {

class Crash
{
public:
    // -> SIGSEGV
    static void nullptrDereference();
    // -> std::terminate()
    static void throwException();
};

}
}
