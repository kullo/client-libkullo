/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <mutex>

#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Util {

template<typename T, typename Mutex>
std::shared_ptr<T> copyGuardedByMutex(std::shared_ptr<T> &original, Mutex &mutex)
{
    // lock mutex, copy listener_, unlock mutex, return copy
    std::lock_guard<Mutex> lock(mutex); K_RAII(lock);
    return original;
}

}
}
