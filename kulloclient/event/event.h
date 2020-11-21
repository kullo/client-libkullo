/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/api/InternalEvent.h"
#include "kulloclient/event/definitions.h"
#include "kulloclient/event/eventlisteners.h"

namespace Kullo {
namespace Event {

class Event : public Api::InternalEvent
{
public:
    virtual ApiEvents notify(const EventListeners &listeners) = 0;
};

}
}
