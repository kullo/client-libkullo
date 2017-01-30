/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/Senders.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api_impl/sessiondata.h"
#include "kulloclient/dao/participantdao.h"
#include "kulloclient/event/removaleventlistener.h"
#include "kulloclient/event/senderseventlistener.h"

namespace Kullo {
namespace ApiImpl {

class SendersImpl :
        public Api::Senders,
        public Event::RemovalEventListener,
        public Event::SendersEventListener
{
public:
    SendersImpl(
            std::shared_ptr<SessionData> session,
            std::shared_ptr<Api::SessionListener> sessionListener);

    // Api::Senders

    std::string name(int64_t msgId) override;

    std::shared_ptr<Api::Address> address(int64_t msgId) override;

    std::string organization(int64_t msgId) override;

    std::string avatarMimeType(int64_t msgId) override;

    std::vector<uint8_t> avatar(int64_t msgId) override;

    // Event::RemovalEventListener

    Event::ApiEvents messageRemoved(int64_t convId, int64_t msgId) override;

    // Event::SendersEventListener

    Event::ApiEvents senderAdded(int64_t msgId) override;

private:
    std::shared_ptr<SessionData> sessionData_;
    std::shared_ptr<Api::SessionListener> sessionListener_;
    std::map<int64_t, Dao::ParticipantDao> senders_;
    std::map<int64_t, std::vector<unsigned char>> avatars_;
};

}
}
