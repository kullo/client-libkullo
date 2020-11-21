/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/messageattachmentsimpl.h"

#include <algorithm>
#include <smartsqlite/exceptions.h>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api_impl/asynctaskimpl.h"
#include "kulloclient/api_impl/messageattachmentscontentworker.h"
#include "kulloclient/api_impl/messageattachmentssavetoworker.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace ApiImpl {

MessageAttachmentsImpl::MessageAttachmentsImpl(
        std::shared_ptr<SessionData> session,
        std::shared_ptr<Api::SessionListener> sessionListener)
    : sessionData_(session)
    , sessionListener_(sessionListener)
{
    auto daos = Dao::AttachmentDao::all(
                Dao::IsDraft::No,
                sessionData_->dbSession_);
    addAttachmentDaosForNewMessages(std::move(daos));
}

std::vector<int64_t> MessageAttachmentsImpl::allForMessage(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    return attIds_[msgId];
}

bool MessageAttachmentsImpl::allAttachmentsDownloaded(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    try
    {
        return Dao::AttachmentDao::allAttachmentsDownloaded(
                    msgId, sessionData_->dbSession_);
    }
    catch (SmartSqlite::QueryReturnedNoRows &)
    {
        return true;
    }
}

std::string MessageAttachmentsImpl::filename(int64_t msgId, int64_t attId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    auto daoIter = attachments_.find(std::make_pair(msgId, attId));
    return daoIter != attachments_.end() ? daoIter->second.filename() : "";
}

std::string MessageAttachmentsImpl::mimeType(int64_t msgId, int64_t attId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    auto daoIter = attachments_.find(std::make_pair(msgId, attId));
    return daoIter != attachments_.end() ? daoIter->second.mimeType() : "";
}

int64_t MessageAttachmentsImpl::size(int64_t msgId, int64_t attId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    auto daoIter = attachments_.find(std::make_pair(msgId, attId));
    return daoIter != attachments_.end() ? daoIter->second.size() : 0;
}

std::string MessageAttachmentsImpl::hash(int64_t msgId, int64_t attId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    auto daoIter = attachments_.find(std::make_pair(msgId, attId));
    return daoIter != attachments_.end() ? daoIter->second.hash() : "";
}

nn_shared_ptr<Api::AsyncTask> MessageAttachmentsImpl::contentAsync(
        int64_t msgId, int64_t attId,
        const nn_shared_ptr<Api::MessageAttachmentsContentListener> &listener)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    return nn_make_shared<AsyncTaskImpl>(
                std::make_shared<MessageAttachmentsContentWorker>(
                    msgId, attId, sessionData_->sessionConfig_, listener));
}

nn_shared_ptr<Api::AsyncTask> MessageAttachmentsImpl::saveToAsync(
        int64_t msgId, int64_t attId, const std::string &path,
        const nn_shared_ptr<Api::MessageAttachmentsSaveToListener> &listener)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);
    kulloAssert(!path.empty());

    return nn_make_shared<AsyncTaskImpl>(
                std::make_shared<MessageAttachmentsSaveToWorker>(
                    msgId, attId, path, sessionData_->sessionConfig_, listener));
}

Event::ApiEvents MessageAttachmentsImpl::messageAdded(
        int64_t convId, int64_t msgId)
{
    K_UNUSED(convId);

    kulloAssert(attIds_[msgId].empty());

    auto daos = Dao::AttachmentDao::allForMessage(
                Dao::IsDraft::No,
                msgId,
                sessionData_->dbSession_);

    addAttachmentDaosForNewMessages(std::move(daos));
    return {{}};
}

Event::ApiEvents MessageAttachmentsImpl::messageAttachmentsDownloaded(
        uint64_t msgId)
{
    return {Api::Event(
                    Api::EventType::MessageAttachmentsDownloadedChanged,
                    -1, msgId, -1)};
}

Event::ApiEvents MessageAttachmentsImpl::messageRemoved(
        int64_t convId, int64_t msgId)
{
    K_UNUSED(convId);

    for (auto attId : allForMessage(msgId))
    {
        auto daoIter = attachments_.find(std::make_pair(msgId, attId));
        if (daoIter != attachments_.end())
        {
            SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
            daoIter->second.deletePermanently();
            tx.commit();
            attachments_.erase(daoIter);

            auto &idsForMsg = attIds_[msgId];
            auto idIter = std::find(idsForMsg.begin(), idsForMsg.end(), attId);
            kulloAssert(idIter != idsForMsg.end());
            idsForMsg.erase(idIter);
        }
    }

    // We don't return some (non-existing) MessageAttachmentDeleted signal here,
    // because attachments can only be removed together with messages, which
    // have such a signal.
    //
    // "{{}}" forces initializer list constructor
    // http://stackoverflow.com/questions/26947704/implicit-conversion-failure-from-initializer-list
    return {{}};
}

void MessageAttachmentsImpl::addAttachmentDaosForNewMessages(
        std::unique_ptr<Dao::AttachmentResult> daos)
{
    while (const auto &dao = daos->next())
    {
        attIds_[dao->messageId()].emplace_back(dao->index());
        attachments_.emplace_hint(
                    attachments_.end(),
                    std::make_pair(dao->messageId(), dao->index()),
                    *dao);
    }
}

}
}
