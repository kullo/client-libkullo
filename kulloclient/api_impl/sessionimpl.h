/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <thread>

#include "kulloclient/api/Session.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api/Syncer.h"
#include "kulloclient/api_impl/conversationsimpl.h"
#include "kulloclient/api_impl/draftsimpl.h"
#include "kulloclient/api_impl/draftattachmentsimpl.h"
#include "kulloclient/api_impl/messageattachmentsimpl.h"
#include "kulloclient/api_impl/messageseventdispatcher.h"
#include "kulloclient/api_impl/messagesimpl.h"
#include "kulloclient/api_impl/removaleventdispatcher.h"
#include "kulloclient/api_impl/sendersimpl.h"
#include "kulloclient/api_impl/sessiondata.h"
#include "kulloclient/event/eventlisteners.h"

namespace Kullo {
namespace ApiImpl {

class SessionImpl : public Api::Session, public Event::EventListeners
{
public:
    SessionImpl(
            const Db::SessionConfig sessionConfig,
            Db::SharedSessionPtr dbSession,
            std::shared_ptr<Api::SessionListener> listener,
            const boost::optional<std::thread::id> mainThread);

    // Api::Session impl
    std::shared_ptr<Api::UserSettings> userSettings() override;
    std::shared_ptr<Api::Conversations> conversations() override;
    std::shared_ptr<Api::Messages> messages() override;
    std::shared_ptr<Api::MessageAttachments> messageAttachments() override;
    std::shared_ptr<Api::Senders> senders() override;
    std::shared_ptr<Api::Drafts> drafts() override;
    std::shared_ptr<Api::DraftAttachments> draftAttachments() override;
    std::shared_ptr<Api::Syncer> syncer() override;
    std::shared_ptr<Api::AsyncTask> accountInfoAsync(
            const std::shared_ptr<Api::SessionAccountInfoListener> &listener) override;
    std::shared_ptr<Api::AsyncTask> registerPushToken(
            const Api::PushToken &token) override;
    std::shared_ptr<Api::AsyncTask> unregisterPushToken(
            const Api::PushToken &token) override;
    std::vector<Api::Event> notify(
            const std::shared_ptr<Api::InternalEvent> &internalEvent) override;

    // Event::EventListeners impl
    Event::ConversationsEventListener &conversationsEventListener() const override;
    Event::MessagesEventListener &messagesEventListener() const override;
    Event::SendersEventListener &sendersEventListener() const override;
    Event::MessageAttachmentsEventListener &messageAttachmentsEventListener() const override;
    Event::DraftsEventListener &draftsEventListener() const override;
    Event::DraftAttachmentsEventListener &draftAttachmentsEventListener() const override;
    Event::UserSettingsEventListener &userSettingsEventListener() const override;
    Event::RemovalEventListener &removalEventListener() const override;

private:
    std::shared_ptr<SessionData> sessionData_;
    std::shared_ptr<ConversationsImpl> conversations_;
    std::shared_ptr<MessagesImpl> messages_;
    std::shared_ptr<MessageAttachmentsImpl> messageAttachments_;
    std::shared_ptr<SendersImpl> senders_;
    std::shared_ptr<DraftsImpl> drafts_;
    std::shared_ptr<DraftAttachmentsImpl> draftAttachments_;
    std::shared_ptr<Api::Syncer> syncer_;
    std::shared_ptr<Api::SessionListener> listener_;
    const boost::optional<std::thread::id> mainThread_;

    friend class MessagesEventDispatcher;
    std::unique_ptr<MessagesEventDispatcher> messagesEventDispatcher_;
    friend class RemovalEventDispatcher;
    std::unique_ptr<RemovalEventDispatcher> removalEventDispatcher_;
};

}
}
