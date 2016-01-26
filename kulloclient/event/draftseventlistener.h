/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/definitions.h"

namespace Kullo {
namespace Event {

class DraftsEventListener
{
public:
    virtual ApiEvents draftModified(uint64_t convId) = 0;
};

}
}
