/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/draftsimpl.h"

#include <stdexcept>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api/DraftState.h"
#include "kulloclient/api/Event.h"
#include "kulloclient/api/EventType.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/event/draftremovedevent.h"
#include "kulloclient/event/sendapieventsevent.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"

namespace Kullo {
namespace ApiImpl {

DraftsImpl::DraftsImpl(
        std::shared_ptr<SessionData> session,
        std::shared_ptr<Api::SessionListener> sessionListener)
    : sessionData_(session)
    , sessionListener_(sessionListener)
{
    auto drafts = Dao::DraftDao::all(sessionData_->dbSession_);
    while (const auto &dao = drafts->next())
    {
        drafts_.emplace_hint(drafts_.end(), dao->conversationId(), *dao);
    }
}

std::string DraftsImpl::text(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto draftIter = drafts_.find(convId);
    return draftIter != drafts_.end() ? draftIter->second.text() : "";
}

void DraftsImpl::setText(int64_t convId, const std::string &text)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    bool changed = false;

    auto draftIter = drafts_.find(convId);
    if (draftIter != drafts_.end())
    {
        if (draftIter->second.text() != text)
        {
            SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
            draftIter->second.setText(text);
            draftIter->second.save();
            tx.commit();
            changed = true;
        }
    }
    else
    {
        SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
        Dao::DraftDao dao(sessionData_->dbSession_);
        dao.setConversationId(convId);
        dao.setText(text);
        dao.save();
        tx.commit();

        drafts_.emplace(convId, std::move(dao));
        changed = true;
    }
    if (changed)
    {
        Api::Event event(Api::EventType::DraftTextChanged, convId, -1, -1);
        sessionListener_->internalEvent(
                    nn_make_shared<Event::SendApiEventsEvent>(event));
    }
}

Api::DraftState DraftsImpl::state(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto draftIter = drafts_.find(convId);
    if (draftIter == drafts_.end()) return Api::DraftState::Editing;

    switch (draftIter->second.state())
    {
    case Dao::DraftState::Editing: return Api::DraftState::Editing;
    case Dao::DraftState::Sending: return Api::DraftState::Sending;
    }
}

void DraftsImpl::prepareToSend(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto draftIter = drafts_.find(convId);
    if (draftIter == drafts_.end())
    {
        Dao::DraftDao dao(sessionData_->dbSession_);
        dao.setConversationId(convId);
        draftIter = drafts_.emplace(convId, std::move(dao)).first;
    }

    Dao::DraftDao &dao = draftIter->second;
    auto &userSettings = sessionData_->userSettings_;

    auto senderName = userSettings->name();
    if (senderName.empty())
    {
        throw std::logic_error("UserSettings::name() must be non-empty");
    }

    SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
    dao.setState(Dao::DraftState::Sending);
    dao.setFooter(userSettings->footer());
    dao.setSenderName(senderName);
    dao.setSenderOrganization(userSettings->organization());
    dao.setSenderAvatar(userSettings->avatar());
    dao.setSenderAvatarMimeType(userSettings->avatarMimeType());
    dao.save();
    tx.commit();

    Api::Event event(Api::EventType::DraftStateChanged, convId, -1, -1);
    sessionListener_->internalEvent(
                nn_make_shared<Event::SendApiEventsEvent>(event));
}

void DraftsImpl::clear(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto draftIter = drafts_.find(convId);
    if (draftIter != drafts_.end())
    {
        SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
        Dao::DraftDao &dao = draftIter->second;
        dao.clear();
        dao.save();
        tx.commit();

        sessionListener_->internalEvent(
                    nn_make_shared<Event::DraftRemovedEvent>(convId));
    }
}

Event::ApiEvents DraftsImpl::draftModified(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto dao = Dao::DraftDao::load(convId, sessionData_->dbSession_);
    if (!dao) throw Db::DatabaseIntegrityError("DraftsImpl::draftModified");

    boost::optional<Dao::DraftState> oldState;
    boost::optional<std::string> oldText;

    auto pos = drafts_.find(convId);
    if (pos != drafts_.end())
    {
        oldState = pos->second.state();
        oldText = pos->second.text();
        drafts_.emplace_hint(drafts_.erase(pos), convId, *dao);
    }
    else
    {
        drafts_.emplace(convId, *dao);
    }

    Event::ApiEvents result;
    if (!oldState || oldState != dao->state())
    {
        result.insert({Api::EventType::DraftStateChanged, convId, -1, -1});
    }
    if (!oldText || oldText != dao->text())
    {
        result.insert({Api::EventType::DraftTextChanged, convId, -1, -1});
    }
    return result;
}

Event::ApiEvents DraftsImpl::conversationWillBeRemoved(int64_t convId)
{
    Event::ApiEvents result;

    if (drafts_.find(convId) != drafts_.end())
    {
        clear(convId);
        result.insert({Api::EventType::DraftStateChanged, convId, -1, -1});
        result.insert({Api::EventType::DraftTextChanged, convId, -1, -1});
    }
    return result;
}

}
}
