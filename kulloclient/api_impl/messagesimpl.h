/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <map>

#include "kulloclient/api/Messages.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api_impl/sessiondata.h"
#include "kulloclient/dao/messagedao.h"
#include "kulloclient/event/messageseventlistener.h"
#include "kulloclient/event/removaleventlistener.h"

namespace Kullo {
namespace ApiImpl {

class MessagesImpl :
        public Api::Messages,
        public Event::MessagesEventListener,
        public Event::RemovalEventListener
{
public:
    MessagesImpl(
            std::shared_ptr<SessionData> session,
            std::shared_ptr<Api::SessionListener> sessionListener);

    // Api::Messages

    std::vector<int64_t> allForConversation(int64_t convId) override;

    int64_t latestForSender(const std::shared_ptr<Api::Address> & address) override;

    void remove(int64_t msgId) override;

    int64_t conversation(int64_t msgId) override;

    std::vector<std::shared_ptr<Api::Delivery>> deliveryState(
            int64_t msgId) override;

    bool isRead(int64_t msgId) override;

    void setRead(int64_t msgId, bool value) override;

    bool isDone(int64_t msgId) override;

    void setDone(int64_t msgId, bool value) override;

    Api::DateTime dateSent(int64_t msgId) override;

    Api::DateTime dateReceived(int64_t msgId) override;

    std::string text(int64_t msgId) override;

    std::string textAsHtml(int64_t msgId, bool includeKulloAddresses) override;

    std::string footer(int64_t msgId) override;

    // Event::MessagesEventListener

    Event::ApiEvents messageAdded(int64_t convId, int64_t msgId) override;
    Event::ApiEvents messageModified(int64_t convId, int64_t msgId) override;

    // Event::RemovalEventListener

    Event::ApiEvents conversationWillBeRemoved(int64_t convId) override;
    Event::ApiEvents messageRemoved(int64_t convId, int64_t msgId) override;

private:
    enum struct MessageState {
        Read = 0,
        Done
    };

    void setState(MessageState state, int64_t msgId, bool value);

    /**
     * @brief Removes the given message from the data structures of this class.
     * @return The removed DAO, if one has been found
     */
    boost::optional<Dao::MessageDao> removeFromCache(int64_t msgId);
    Event::ApiEvents removeFromDb(Dao::MessageDao &dao);

    std::shared_ptr<SessionData> sessionData_;
    std::shared_ptr<Api::SessionListener> sessionListener_;
    std::map<int64_t, std::vector<int64_t>> idsForConvId_;
    std::map<int64_t, Dao::MessageDao> messages_;
    std::map<int64_t, std::vector<std::shared_ptr<Api::Delivery>>> delivery_;
};

}
}
