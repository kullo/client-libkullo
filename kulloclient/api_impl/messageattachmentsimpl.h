/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <map>

#include "kulloclient/api/MessageAttachments.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api_impl/sessiondata.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/event/messageattachmentseventlistener.h"
#include "kulloclient/event/messageseventlistener.h"
#include "kulloclient/event/removaleventlistener.h"

namespace Kullo {
namespace ApiImpl {

class MessageAttachmentsImpl :
        public Api::MessageAttachments,
        public Event::MessagesEventListener,
        public Event::MessageAttachmentsEventListener,
        public Event::RemovalEventListener
{
public:
    MessageAttachmentsImpl(
            std::shared_ptr<SessionData> session,
            std::shared_ptr<Api::SessionListener> sessionListener);

    // Api::MessageAttachments

    std::vector<int64_t> allForMessage(int64_t msgId) override;

    bool allAttachmentsDownloaded(int64_t msgId) override;

    std::string filename(int64_t msgId, int64_t attId) override;

    std::string mimeType(int64_t msgId, int64_t attId) override;

    int64_t size(int64_t msgId, int64_t attId) override;

    std::string hash(int64_t msgId, int64_t attId) override;

    std::shared_ptr<Api::AsyncTask> contentAsync(
            int64_t msgId, int64_t attId,
            const std::shared_ptr<Api::MessageAttachmentsContentListener> &listener
            ) override;

    std::shared_ptr<Api::AsyncTask> saveToAsync(
            int64_t msgId, int64_t attId, const std::string &path,
            const std::shared_ptr<Api::MessageAttachmentsSaveToListener> &listener
            ) override;

    // Event::MessagesEventListener

    Event::ApiEvents messageAdded(int64_t convId, int64_t msgId) override;

    // Event::MessageAttachmentsEventListener

    Event::ApiEvents messageAttachmentsDownloaded(uint64_t msgId) override;

    // Event::RemovalEventListener

    Event::ApiEvents messageRemoved(int64_t convId, int64_t msgId) override;

private:
    void addAttachmentDaosForNewMessages(
            std::unique_ptr<Dao::AttachmentResult> daos);

    std::shared_ptr<SessionData> sessionData_;
    std::shared_ptr<Api::SessionListener> sessionListener_;
    std::map<int64_t, std::vector<int64_t>> attIds_;
    std::map<std::pair<int64_t, int64_t>, Dao::AttachmentDao> attachments_;
};

}
}
