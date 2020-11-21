/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <map>

#include "kulloclient/api/DraftAttachments.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api_impl/sessiondata.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/event/draftattachmentseventlistener.h"
#include "kulloclient/event/removaleventlistener.h"

namespace Kullo {
namespace ApiImpl {

class DraftAttachmentsImpl :
        public Api::DraftAttachments,
        public Event::DraftAttachmentsEventListener,
        public Event::RemovalEventListener
{
public:
    DraftAttachmentsImpl(
            std::shared_ptr<SessionData> session,
            std::shared_ptr<Api::SessionListener> sessionListener);

    // Api::DraftAttachments impl

    std::vector<int64_t> allForDraft(int64_t convId) override;

    nn_shared_ptr<Api::AsyncTask> addAsync(
            int64_t convId,
            const std::string &path,
            const std::string &mimeType,
            const nn_shared_ptr<Api::DraftAttachmentsAddListener> &listener
            ) override;

    void remove(int64_t convId, int64_t attId) override;

    std::string filename(int64_t convId, int64_t attId) override;

    void setFilename(
            int64_t convId,
            int64_t attId,
            const std::string &filename) override;

    std::string mimeType(int64_t convId, int64_t attId) override;

    int64_t size(int64_t convId, int64_t attId) override;

    std::string hash(int64_t convId, int64_t attId) override;

    nn_shared_ptr<Api::AsyncTask> contentAsync(
            int64_t convId,
            int64_t attId,
            const nn_shared_ptr<Api::DraftAttachmentsContentListener> &listener
            ) override;

    nn_shared_ptr<Api::AsyncTask> saveToAsync(
            int64_t convId,
            int64_t attId,
            const std::string &path,
            const nn_shared_ptr<Api::DraftAttachmentsSaveToListener> &listener
            ) override;

    // Event::DraftAttachmentsEventListener impl

    Event::ApiEvents draftAttachmentAdded(
            const Data::DraftAttachment &attachment) override;
    Event::ApiEvents draftAttachmentRemoved(
            id_type convId, id_type attId) override;

    // Event::RemovalEventListener impl

    Event::ApiEvents draftRemoved(int64_t convId) override;

private:
    enum struct AlsoRemoveFromDb { No, Yes };

    Event::ApiEvents doRemove(
            id_type convId, id_type attId, AlsoRemoveFromDb removeFromDb);

    std::shared_ptr<SessionData> sessionData_;
    std::shared_ptr<Api::SessionListener> sessionListener_;
    std::map<int64_t, std::vector<int64_t>> attIds_;
    std::map<std::pair<int64_t, int64_t>, Dao::AttachmentDao> attachments_;
};

}
}
