/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/definitions.h"

namespace Kullo {
namespace Event {

class DraftsEventListener
{
public:
    virtual ~DraftsEventListener() = default;
    virtual ApiEvents draftModified(int64_t convId) = 0;
};

}
}
