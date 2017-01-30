/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
