/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
