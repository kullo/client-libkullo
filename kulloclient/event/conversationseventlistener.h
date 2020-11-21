/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/event/definitions.h"

namespace Kullo {
namespace Event {

class ConversationsEventListener
{
public:
    virtual ~ConversationsEventListener() = default;
    virtual ApiEvents conversationAdded(int64_t convId) = 0;
    virtual ApiEvents conversationModified(int64_t convId) = 0;
};

}
}
