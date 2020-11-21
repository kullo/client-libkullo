/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/event/removaleventlistener.h"

namespace Kullo {
namespace ApiImpl {

class SessionImpl;

class RemovalEventDispatcher : public Event::RemovalEventListener
{
public:
    RemovalEventDispatcher(SessionImpl &session);

    Event::ApiEvents conversationWillBeRemoved(int64_t convId) override;
    Event::ApiEvents messageRemoved(int64_t convId, int64_t msgId) override;
    Event::ApiEvents draftRemoved(int64_t convId) override;

private:
    SessionImpl &session_;
};

}
}
