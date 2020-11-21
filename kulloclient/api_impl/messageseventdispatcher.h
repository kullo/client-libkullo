/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/event/messageseventlistener.h"

namespace Kullo {
namespace ApiImpl {

class SessionImpl;

class MessagesEventDispatcher : public Event::MessagesEventListener
{
public:
    MessagesEventDispatcher(SessionImpl &session);

    Event::ApiEvents messageAdded(int64_t convId, int64_t msgId) override;
    Event::ApiEvents messageModified(int64_t convId, int64_t msgId) override;

private:
    SessionImpl &session_;
};

}
}
