/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/sessionimpl.h"

#include <algorithm>
#include <unordered_set>

#include "kulloclient/api/Address.h"
#include "kulloclient/api/MasterKey.h"
#include "kulloclient/api_impl/asynctaskimpl.h"
#include "kulloclient/api_impl/conversationsimpl.h"
#include "kulloclient/api_impl/draftsimpl.h"
#include "kulloclient/api_impl/draftattachmentsimpl.h"
#include "kulloclient/api_impl/event.h"
#include "kulloclient/api_impl/messagesimpl.h"
#include "kulloclient/api_impl/messageattachmentsimpl.h"
#include "kulloclient/api_impl/sendersimpl.h"
#include "kulloclient/api_impl/sessionaccountinfoworker.h"
#include "kulloclient/api_impl/sessionregisterpushtokenworker.h"
#include "kulloclient/api_impl/syncerimpl.h"
#include "kulloclient/api_impl/usersettingsimpl.h"
#include "kulloclient/event/event.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/util/masterkey.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace ApiImpl {

SessionImpl::SessionImpl(
        const std::string &dbPath,
        Db::SharedSessionPtr dbSession,
        std::shared_ptr<UserSettingsImpl> userSettings,
        std::shared_ptr<Api::SessionListener> listener)
    : sessionData_(
          std::make_shared<SessionData>(dbPath, dbSession, userSettings))
    , conversations_(
          std::make_shared<ConversationsImpl>(sessionData_, listener))
    , messages_(
          std::make_shared<MessagesImpl>(sessionData_, listener))
    , messageAttachments_(
          std::make_shared<MessageAttachmentsImpl>(sessionData_, listener))
    , senders_(
          std::make_shared<SendersImpl>(sessionData_, listener))
    , drafts_(
          std::make_shared<DraftsImpl>(sessionData_, listener))
    , draftAttachments_(
          std::make_shared<DraftAttachmentsImpl>(sessionData_, listener))
    , syncer_(
          std::make_shared<SyncerImpl>(sessionData_, listener))
    , listener_(listener)
    , messagesEventDispatcher_(make_unique<MessagesEventDispatcher>(*this))
    , removalEventDispatcher_(make_unique<RemovalEventDispatcher>(*this))
{}

std::shared_ptr<Api::UserSettings> SessionImpl::userSettings()
{
    return sessionData_->userSettings_;
}

std::shared_ptr<Api::Conversations> SessionImpl::conversations()
{
    return conversations_;
}

std::shared_ptr<Api::Messages> SessionImpl::messages()
{
    return messages_;
}

std::shared_ptr<Api::MessageAttachments> SessionImpl::messageAttachments()
{
    return messageAttachments_;
}

std::shared_ptr<Api::Senders> SessionImpl::senders()
{
    return senders_;
}

std::shared_ptr<Api::Drafts> SessionImpl::drafts()
{
    return drafts_;
}

std::shared_ptr<Api::DraftAttachments> SessionImpl::draftAttachments()
{
    return draftAttachments_;
}

std::shared_ptr<Api::Syncer> SessionImpl::syncer()
{
    return syncer_;
}

std::shared_ptr<Api::AsyncTask> SessionImpl::accountInfoAsync(
        const std::shared_ptr<Api::SessionAccountInfoListener> &listener)
{
    kulloAssert(listener);

    auto &userSettings = sessionData_->userSettings_;
    auto address = Util::KulloAddress(userSettings->address()->toString());
    auto masterKey = Util::MasterKey(userSettings->masterKey()->dataBlocks());

    return std::make_shared<AsyncTaskImpl>(
        std::make_shared<SessionAccountInfoWorker>(
                    address, masterKey, listener));
}

std::shared_ptr<Api::AsyncTask> SessionImpl::registerPushToken(
        const std::string &registrationToken)
{
    kulloAssert(!registrationToken.empty());

    auto &userSettings = sessionData_->userSettings_;
    auto address = Util::KulloAddress(userSettings->address()->toString());
    auto masterKey = Util::MasterKey(userSettings->masterKey()->dataBlocks());

    return std::make_shared<AsyncTaskImpl>(
        std::make_shared<SessionRegisterPushTokenWorker>(
                    address, masterKey,
                    SessionRegisterPushTokenWorker::Add, registrationToken));
}

std::shared_ptr<Api::AsyncTask> SessionImpl::unregisterPushToken(
        const std::string &registrationToken)
{
    kulloAssert(!registrationToken.empty());

    auto &userSettings = sessionData_->userSettings_;
    auto address = Util::KulloAddress(userSettings->address()->toString());
    auto masterKey = Util::MasterKey(userSettings->masterKey()->dataBlocks());

    return std::make_shared<AsyncTaskImpl>(
        std::make_shared<SessionRegisterPushTokenWorker>(
                    address, masterKey,
                    SessionRegisterPushTokenWorker::Remove, registrationToken));
}

std::vector<Api::Event> SessionImpl::notify(
        const std::shared_ptr<Api::InternalEvent> &internalEvent)
{
    kulloAssert(internalEvent);

    auto event = std::dynamic_pointer_cast<Event::Event>(internalEvent);
    kulloAssert(event);
    auto apiEventSet = event->notify(*this);
    auto apiEventVector = std::vector<Api::Event>(
                apiEventSet.begin(), apiEventSet.end());
    std::sort(apiEventVector.begin(), apiEventVector.end());
    return apiEventVector;
}

Event::ConversationsEventListener &SessionImpl::conversationsEventListener() const
{
    return *conversations_;
}

Event::MessagesEventListener &SessionImpl::messagesEventListener() const
{
    return *messagesEventDispatcher_;
}

Event::SendersEventListener &SessionImpl::sendersEventListener() const
{
    return *senders_;
}

Event::MessageAttachmentsEventListener &SessionImpl::messageAttachmentsEventListener() const
{
    return *messageAttachments_;
}

Event::DraftsEventListener &SessionImpl::draftsEventListener() const
{
    return *drafts_;
}

Event::DraftAttachmentsEventListener &SessionImpl::draftAttachmentsEventListener() const
{
    return *draftAttachments_;
}

Event::RemovalEventListener &SessionImpl::removalEventListener() const
{
    return *removalEventDispatcher_;
}

}
}
