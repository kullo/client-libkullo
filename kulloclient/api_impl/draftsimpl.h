/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <map>

#include "kulloclient/api/Drafts.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api_impl/sessiondata.h"
#include "kulloclient/dao/draftdao.h"
#include "kulloclient/event/draftseventlistener.h"
#include "kulloclient/event/removaleventlistener.h"

namespace Kullo {
namespace ApiImpl {

class DraftsImpl :
        public Api::Drafts,
        public Event::DraftsEventListener,
        public Event::RemovalEventListener
{
public:
    DraftsImpl(
            std::shared_ptr<SessionData> session,
            std::shared_ptr<Api::SessionListener> sessionListener);

    // Api::Drafts

    std::string text(int64_t convId) override;
    void setText(int64_t convId, const std::string &text) override;
    Api::DraftState state(int64_t convId) override;
    void prepareToSend(int64_t convId) override;
    void clear(int64_t convId) override;

    // Event::DraftsEventListener

    Event::ApiEvents draftModified(uint64_t convId) override;

    // Event::RemovalEventListener

    Event::ApiEvents conversationWillBeRemoved(int64_t convId) override;

private:
    std::shared_ptr<SessionData> sessionData_;
    std::shared_ptr<Api::SessionListener> sessionListener_;
    std::map<int64_t, Dao::DraftDao> drafts_;
};

}
}
