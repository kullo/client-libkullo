/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
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
