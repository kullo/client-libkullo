/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/draftattachmentsimpl.h"

#include <algorithm>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api_impl/asynctaskimpl.h"
#include "kulloclient/api_impl/draftattachmentsaddworker.h"
#include "kulloclient/api_impl/draftattachmentscontentworker.h"
#include "kulloclient/api_impl/draftattachmentssavetoworker.h"
#include "kulloclient/event/sendapieventsevent.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

DraftAttachmentsImpl::DraftAttachmentsImpl(
        std::shared_ptr<SessionData> session,
        std::shared_ptr<Api::SessionListener> sessionListener)
    : sessionData_(session)
    , sessionListener_(sessionListener)
{
    auto daos = Dao::AttachmentDao::all(
                Dao::IsDraft::Yes,
                sessionData_->dbSession_);
    while (const auto &dao = daos->next())
    {
        attIds_[dao->messageId()].emplace_back(dao->index());
        attachments_.emplace_hint(
                    attachments_.end(),
                    std::make_pair(dao->messageId(), dao->index()),
                    *dao);
    }
}

std::vector<int64_t> DraftAttachmentsImpl::allForDraft(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    return attIds_[convId];
}

nn_shared_ptr<Api::AsyncTask> DraftAttachmentsImpl::addAsync(
        int64_t convId,
        const std::string &path,
        const std::string &mimeType,
        const nn_shared_ptr<Api::DraftAttachmentsAddListener> &listener)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    kulloAssert(!path.empty());
    kulloAssert(!mimeType.empty());

    return nn_make_shared<AsyncTaskImpl>(
                std::make_shared<DraftAttachmentsAddWorker>(
                    convId, path, mimeType, sessionData_->sessionConfig_,
                    sessionListener_, listener));
}

void DraftAttachmentsImpl::remove(int64_t convId, int64_t attId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    auto events = doRemove(convId, attId, AlsoRemoveFromDb::Yes);

    if (!events.empty())
    {
        sessionListener_->internalEvent(
                    nn_make_shared<Event::SendApiEventsEvent>(events));
    }
}

std::string DraftAttachmentsImpl::filename(int64_t convId, int64_t attId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    auto daoIter = attachments_.find(std::make_pair(convId, attId));
    return daoIter != attachments_.end() ? daoIter->second.filename() : "";
}

void DraftAttachmentsImpl::setFilename(
        int64_t convId, int64_t attId, const std::string &filename)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);
    kulloAssert(!filename.empty());

    auto daoIter = attachments_.find(std::make_pair(convId, attId));
    if (daoIter != attachments_.end())
    {
        auto &dao = daoIter->second;

        SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
        dao.setFilename(filename);
        dao.save();
        tx.commit();
    }
}

std::string DraftAttachmentsImpl::mimeType(int64_t convId, int64_t attId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    auto daoIter = attachments_.find(std::make_pair(convId, attId));
    return daoIter != attachments_.end() ? daoIter->second.mimeType() : "";
}

int64_t DraftAttachmentsImpl::size(int64_t convId, int64_t attId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    auto daoIter = attachments_.find(std::make_pair(convId, attId));
    return daoIter != attachments_.end() ? daoIter->second.size() : 0;
}

std::string DraftAttachmentsImpl::hash(int64_t convId, int64_t attId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    auto daoIter = attachments_.find(std::make_pair(convId, attId));
    return daoIter != attachments_.end() ? daoIter->second.hash() : "";
}

nn_shared_ptr<Api::AsyncTask> DraftAttachmentsImpl::contentAsync(
        int64_t convId,
        int64_t attId,
        const nn_shared_ptr<Api::DraftAttachmentsContentListener> &listener)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);

    return nn_make_shared<AsyncTaskImpl>(
                std::make_shared<DraftAttachmentsContentWorker>(
                    convId, attId, sessionData_->sessionConfig_, listener));
}

nn_shared_ptr<Api::AsyncTask> DraftAttachmentsImpl::saveToAsync(int64_t convId,
        int64_t attId,
        const std::string &path,
        const nn_shared_ptr<Api::DraftAttachmentsSaveToListener> &listener)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    kulloAssert(attId >= Kullo::ID_MIN && attId <= Kullo::ID_MAX);
    kulloAssert(!path.empty());

    return nn_make_shared<AsyncTaskImpl>(
                std::make_shared<DraftAttachmentsSaveToWorker>(
                    convId, attId, path, sessionData_->sessionConfig_, listener));
}

Event::ApiEvents DraftAttachmentsImpl::draftAttachmentAdded(
        const Data::DraftAttachment &attachment)
{
    auto dao = Dao::AttachmentDao::load(
                Dao::IsDraft::Yes,
                attachment.convId,
                attachment.attId,
                sessionData_->dbSession_);
    if (!dao) throw Db::DatabaseIntegrityError(
                "DraftAttachmentsImpl::draftAttachmentAdded: "
                "Couldn't load attachment ("
                "conversationId: " + std::to_string(attachment.convId) + "; "
                "attachmentId: " + std::to_string(attachment.attId) + ")");

    attIds_[attachment.convId].emplace_back(attachment.attId);
    attachments_.emplace(
                std::make_pair(attachment.convId, attachment.attId),
                *dao);

    return {Api::Event(
                    Api::EventType::DraftAttachmentAdded,
                    attachment.convId,
                    -1,
                    attachment.attId)};
}

Event::ApiEvents DraftAttachmentsImpl::draftAttachmentRemoved(
        id_type convId, id_type attId)
{
    return doRemove(convId, attId, AlsoRemoveFromDb::No);
}

//FIXME convert to notification (is sent by Drafts model)
Event::ApiEvents DraftAttachmentsImpl::draftRemoved(int64_t convId)
{
    Event::ApiEvents result;

    for (auto attId : allForDraft(convId))
    {
        auto events = doRemove(convId, attId, AlsoRemoveFromDb::Yes);
        result.insert(events.begin(), events.end());
    }
    return result;
}

Event::ApiEvents DraftAttachmentsImpl::doRemove(
        id_type convId, id_type attId, AlsoRemoveFromDb removeFromDb)
{
    Event::ApiEvents result;

    auto daoIter = attachments_.find(std::make_pair(convId, attId));
    if (daoIter != attachments_.end())
    {
        if (removeFromDb == AlsoRemoveFromDb::Yes)
        {
            SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
            daoIter->second.deletePermanently();
            tx.commit();
        }
        attachments_.erase(daoIter);

        auto &idsForConv = attIds_[convId];
        auto idIter = std::find(idsForConv.begin(), idsForConv.end(), attId);
        kulloAssert(idIter != idsForConv.end());
        idsForConv.erase(idIter);

        result.emplace(
                    Api::Event(
                        Api::EventType::DraftAttachmentRemoved,
                        convId, -1, attId));
    }
    return result;
}

}
}
