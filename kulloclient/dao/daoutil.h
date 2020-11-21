/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <cstdint>

namespace Kullo {
namespace Dao {

// an empty message has about 23.5 kB
const std::int64_t PER_MESSAGE_OVERHEAD = 23500;

template <typename T>
bool assignAndUpdateDirty(T &member, const T &newValue, bool &dirty)
{
    if (member == newValue) return false;

    member = newValue;
    dirty = true;
    return true;
}

}
}
