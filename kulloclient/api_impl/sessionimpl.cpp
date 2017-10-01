/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/sessionimpl.h"

#include <algorithm>
#include <unordered_set>

#include "kulloclient/api/PushToken.h"
#include "kulloclient/api_impl/asynctaskimpl.h"
#include "kulloclient/api_impl/conversationsimpl.h"
#include "kulloclient/api_impl/draftsimpl.h"
#include "kulloclient/api_impl/draftattachmentsimpl.h"
#include "kulloclient/api_impl/event.h"
#include "kulloclient/api_impl/MasterKey.h"
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
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/masterkey.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace ApiImpl {

SessionImpl::SessionImpl(
        const Db::SessionConfig sessionConfig,
        Db::SharedSessionPtr dbSession,
        std::shared_ptr<Api::SessionListener> listener,
        const boost::optional<std::thread::id> mainThread)
    : sessionData_(
          std::make_shared<SessionData>(
              sessionConfig,
              dbSession,
              std::make_shared<UserSettingsImpl>(
                  dbSession, sessionConfig.credentials)))
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
    , mainThread_(mainThread)
    , messagesEventDispatcher_(make_unique<MessagesEventDispatcher>(*this))
    , removalEventDispatcher_(make_unique<RemovalEventDispatcher>(*this))
{}

nn_shared_ptr<Api::UserSettings> SessionImpl::userSettings()
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called userSettings() from non-main thread";
    }
    return kulloForcedNn(sessionData_->userSettings_);
}

nn_shared_ptr<Api::Conversations> SessionImpl::conversations()
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called conversations() from non-main thread";
    }
    return kulloForcedNn(conversations_);
}

nn_shared_ptr<Api::Messages> SessionImpl::messages()
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called messages() from non-main thread";
    }
    return kulloForcedNn(messages_);
}

nn_shared_ptr<Api::MessageAttachments> SessionImpl::messageAttachments()
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called messageAttachments() from non-main thread";
    }
    return kulloForcedNn(messageAttachments_);
}

nn_shared_ptr<Api::Senders> SessionImpl::senders()
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called senders() from non-main thread";
    }
    return kulloForcedNn(senders_);
}

nn_shared_ptr<Api::Drafts> SessionImpl::drafts()
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called drafts() from non-main thread";
    }
    return kulloForcedNn(drafts_);
}

nn_shared_ptr<Api::DraftAttachments> SessionImpl::draftAttachments()
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called draftAttachments() from non-main thread";
    }
    return kulloForcedNn(draftAttachments_);
}

nn_shared_ptr<Api::Syncer> SessionImpl::syncer()
{
    return kulloForcedNn(syncer_);
}

nn_shared_ptr<Api::AsyncTask> SessionImpl::accountInfoAsync(
        const nn_shared_ptr<Api::SessionAccountInfoListener> &listener)
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called accountInfoAsync() from non-main thread";
    }

    auto &userSettings = sessionData_->userSettings_;
    auto address = Util::KulloAddress(userSettings->address().toString());
    auto masterKey = Util::MasterKey(userSettings->masterKey().blocks);

    return nn_make_shared<AsyncTaskImpl>(
        std::make_shared<SessionAccountInfoWorker>(
                    address, masterKey, listener));
}

namespace {

void validateToken(const Api::PushToken &token)
{
    kulloAssert(token.type == Api::PushTokenType::GCM);
    kulloAssert(!token.token.empty());
    kulloAssert(token.environment == Api::PushTokenEnvironment::Android ||
                token.environment == Api::PushTokenEnvironment::IOS);
}

}

nn_shared_ptr<Api::AsyncTask> SessionImpl::registerPushToken(
        const Api::PushToken &token)
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called registerPushToken() from non-main thread";
    }

    validateToken(token);

    auto &userSettings = sessionData_->userSettings_;
    auto address = Util::KulloAddress(userSettings->address().toString());
    auto masterKey = Util::MasterKey(userSettings->masterKey().blocks);

    return nn_make_shared<AsyncTaskImpl>(
        std::make_shared<SessionRegisterPushTokenWorker>(
                    address, masterKey,
                    SessionRegisterPushTokenWorker::Add, token));
}

nn_shared_ptr<Api::AsyncTask> SessionImpl::unregisterPushToken(
        const Api::PushToken &token)
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called unregisterPushToken() from non-main thread";
    }

    validateToken(token);

    auto &userSettings = sessionData_->userSettings_;
    auto address = Util::KulloAddress(userSettings->address().toString());
    auto masterKey = Util::MasterKey(userSettings->masterKey().blocks);

    return nn_make_shared<AsyncTaskImpl>(
        std::make_shared<SessionRegisterPushTokenWorker>(
                    address, masterKey,
                    SessionRegisterPushTokenWorker::Remove, token));
}

std::vector<Api::Event> SessionImpl::notify(const nn_shared_ptr<Api::InternalEvent> &internalEvent)
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called notify() from non-main thread";
    }

    auto event = std::dynamic_pointer_cast<Event::Event>(internalEvent.as_nullable());
    kulloAssert(event);
    auto apiEventSet = event->notify(*this);
    auto apiEventVector = std::vector<Api::Event>(
                apiEventSet.begin(), apiEventSet.end());
    std::sort(apiEventVector.begin(), apiEventVector.end());
    return apiEventVector;
}

Event::ConversationsEventListener &SessionImpl::conversationsEventListener() const
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called conversationsEventListener() from non-main thread";
    }
    return *conversations_;
}

Event::MessagesEventListener &SessionImpl::messagesEventListener() const
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called messagesEventListener() from non-main thread";
    }
    return *messagesEventDispatcher_;
}

Event::SendersEventListener &SessionImpl::sendersEventListener() const
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called sendersEventListener() from non-main thread";
    }
    return *senders_;
}

Event::MessageAttachmentsEventListener &SessionImpl::messageAttachmentsEventListener() const
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called messageAttachmentsEventListener() from non-main thread";
    }
    return *messageAttachments_;
}

Event::DraftsEventListener &SessionImpl::draftsEventListener() const
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called draftsEventListener() from non-main thread";
    }
    return *drafts_;
}

Event::DraftAttachmentsEventListener &SessionImpl::draftAttachmentsEventListener() const
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called draftAttachmentsEventListener() from non-main thread";
    }
    return *draftAttachments_;
}

Event::UserSettingsEventListener &SessionImpl::userSettingsEventListener() const
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called userSettingsEventListener() from non-main thread";
    }
    return *sessionData_->userSettings_;
}

Event::RemovalEventListener &SessionImpl::removalEventListener() const
{
    if (mainThread_ && *mainThread_ != std::this_thread::get_id())
    {
        Log.w() << "Called removalEventListener() from non-main thread";
    }
    return *removalEventDispatcher_;
}

}
}
