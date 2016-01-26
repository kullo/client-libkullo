/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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

void Crash::throwException()
{
    throw std::runtime_error("error to test breakpad");
}

}
}
