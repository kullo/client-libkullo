/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

namespace Kullo {
namespace Dao {

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
