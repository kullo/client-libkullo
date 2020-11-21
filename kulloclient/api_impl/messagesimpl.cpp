/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/messagesimpl.h"

#include <algorithm>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api_impl/Address.h"
#include "kulloclient/api_impl/asynctaskimpl.h"
#include "kulloclient/api_impl/DateTime.h"
#include "kulloclient/api_impl/deliveryimpl.h"
#include "kulloclient/api_impl/messagessearchworker.h"
#include "kulloclient/dao/deliverydao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/event/conversationmodifiedevent.h"
#include "kulloclient/event/messagemodifiedevent.h"
#include "kulloclient/event/messageremovedevent.h"
#include "kulloclient/event/sendapieventsevent.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/strings.h"

namespace Kullo {
namespace ApiImpl {

MessagesImpl::MessagesImpl(
        std::shared_ptr<SessionData> session,
        std::shared_ptr<Api::SessionListener> sessionListener)
    : sessionData_(session)
    , sessionListener_(sessionListener)
{
    auto daos = Dao::MessageDao::all(sessionData_->dbSession_);
    while (const auto &dao = daos->next())
    {
        idsForConvId_[dao->conversationId()].emplace_back(dao->id());
        messages_.emplace_hint(messages_.end(), dao->id(), *dao);
    }

    auto deliveryRecords = Dao::DeliveryDao(sessionData_->dbSession_).loadAll();
    for (auto &record : deliveryRecords)
    {
        delivery_[record.first].emplace_back(
                    nn_make_shared<DeliveryImpl>(std::move(record.second)));
    }
}

std::vector<int64_t> MessagesImpl::allForConversation(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    return idsForConvId_[convId];
}

int64_t MessagesImpl::latestForSender(const Api::Address &address)
{
    const auto addressString = address.toString();

    // Messages are ordered by id and "latest" means having the greatest id,
    // so we can iterate from the end and use the first hit
    for (auto itr = messages_.crbegin(); itr != messages_.crend(); ++itr)
    {
        const auto &dao = itr->second;
        if (dao.sender() == addressString)
        {
            return dao.id();
        }
    }

    return -1; // not found
}

boost::optional<Dao::MessageDao> MessagesImpl::removeFromCache(int64_t msgId)
{
    boost::optional<Dao::MessageDao> result;

    auto daoIter = messages_.find(msgId);
    if (daoIter != messages_.end())
    {
        auto &dao = daoIter->second;
        auto convId = dao.conversationId();

        auto &idsForConv = idsForConvId_[convId];
        auto idIter = std::find(idsForConv.begin(), idsForConv.end(), msgId);
        kulloAssert(idIter != idsForConv.end());
        idsForConv.erase(idIter);

        result = std::move(dao);
        messages_.erase(daoIter);
    }
    return result;
}

Event::ApiEvents MessagesImpl::removeFromDb(Dao::MessageDao &dao)
{
    auto conversationId = dao.conversationId();
    auto messageId = dao.id();

    SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
    dao.clearData();
    dao.setDeleted(true);
    dao.save(Dao::CreateOld::Yes);
    tx.commit();

    return {
        Api::Event(Api::EventType::MessageRemoved,
                   conversationId, messageId, -1)
    };
}

void MessagesImpl::remove(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto removedDao = removeFromCache(msgId);
    if (removedDao)
    {
        auto conversationId = removedDao->conversationId();
        auto events = removeFromDb(*removedDao);
        sessionListener_->internalEvent(
                    nn_make_shared<Event::SendApiEventsEvent>(events));
        sessionListener_->internalEvent(
                    nn_make_shared<Event::ConversationModifiedEvent>(
                        conversationId));

        sessionListener_->internalEvent(
                    nn_make_shared<Event::MessageRemovedEvent>(
                        conversationId, msgId));
    }
}

int64_t MessagesImpl::conversation(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = messages_.find(msgId);
    return iter != messages_.end()
            ? iter->second.conversationId()
            : -1;
}

std::vector<nn_shared_ptr<Api::Delivery>> MessagesImpl::deliveryState(
        int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = delivery_.find(msgId);
    return iter != delivery_.end()
            ? iter->second
            : std::vector<nn_shared_ptr<Api::Delivery>>();
}

bool MessagesImpl::isRead(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = messages_.find(msgId);
    return iter != messages_.end() ?
                iter->second.state(Dao::MessageState::Read) : false;
}

bool MessagesImpl::setRead(int64_t msgId, bool value)
{
    return setState(MessageState::Read, msgId, value);
}

bool MessagesImpl::isDone(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = messages_.find(msgId);
    return iter != messages_.end() ?
                iter->second.state(Dao::MessageState::Done) : false;
}

bool MessagesImpl::setDone(int64_t msgId, bool value)
{
    return setState(MessageState::Done, msgId, value);
}

namespace {
const std::string DEFAULT_TIME = "2000-01-01T00:00:00Z";
}

Api::DateTime MessagesImpl::dateSent(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = messages_.find(msgId);
    auto dateStr = iter != messages_.end() ? iter->second.dateSent() : DEFAULT_TIME;
    return Api::DateTime(Util::DateTime(dateStr));
}

Api::DateTime MessagesImpl::dateReceived(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = messages_.find(msgId);
    auto dateStr = iter != messages_.end() ? iter->second.dateReceived() : DEFAULT_TIME;
    return Api::DateTime(Util::DateTime(dateStr));
}

std::string MessagesImpl::text(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = messages_.find(msgId);
    return iter != messages_.end() ? iter->second.text() : "";
}

std::string MessagesImpl::textAsHtml(int64_t msgId, bool includeKulloAddresses)
{
    return Util::Strings::messageTextToHtml(text(msgId), includeKulloAddresses);
}

std::string MessagesImpl::footer(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = messages_.find(msgId);
    return iter != messages_.end() ? iter->second.footer() : "";
}

nn_shared_ptr<Api::AsyncTask> MessagesImpl::searchAsync(
        const std::string &searchText,
        int64_t convId,
        const boost::optional<Api::SenderPredicate> &sender,
        int32_t limitResults,
        const boost::optional<std::string> &boundary,
        const nn_shared_ptr<Api::MessagesSearchListener> &listener)
{
    kulloAssert(convId == -1 || (convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX));
    kulloAssert(limitResults > 0);
    kulloAssert(boundary == boost::none || !boundary->empty());

    return nn_make_shared<AsyncTaskImpl>(
                std::make_shared<MessagesSearchWorker>(
                    searchText, convId, sender, limitResults, boundary,
                    sessionData_->sessionConfig_, listener));
}

Event::ApiEvents MessagesImpl::messageAdded(int64_t convId, int64_t msgId)
{
    K_UNUSED(convId);

    auto dao = Dao::MessageDao::load(
                msgId, Dao::Old::No, sessionData_->dbSession_);
    if (!dao) throw Db::DatabaseIntegrityError("MessagesImpl::messageAdded");

    auto &idsForThisConvId = idsForConvId_[dao->conversationId()];
    idsForThisConvId.emplace_back(msgId);
    std::sort(idsForThisConvId.begin(), idsForThisConvId.end());
    messages_.emplace(msgId, *dao);

    auto deliveryRecords =
            Dao::DeliveryDao(sessionData_->dbSession_).loadDelivery(msgId);
    for (auto &record : deliveryRecords)
    {
        delivery_[msgId].emplace_back(
                    nn_make_shared<DeliveryImpl>(std::move(record)));
    }

    return {Api::Event(
                    Api::EventType::MessageAdded,
                    dao->conversationId(), msgId, -1)};
}

Event::ApiEvents MessagesImpl::messageModified(int64_t convId, int64_t msgId)
{
    K_UNUSED(convId);

    auto dao = Dao::MessageDao::load(
                msgId, Dao::Old::No, sessionData_->dbSession_);
    if (!dao) throw Db::DatabaseIntegrityError("MessagesImpl::messageModified");

    bool stateChanged = true;
    auto iter = messages_.find(msgId);
    if (iter != messages_.end())
    {
        auto oldDao = iter->second;
        stateChanged = *dao != oldDao;
        messages_.emplace_hint(messages_.erase(iter), msgId, *dao);
    }
    else
    {
        Log.w() << "messageModified for unknown message; inserting.";
        messages_.emplace(msgId, *dao);
    }

    auto oldDeliveryList = delivery_[msgId];
    delivery_[msgId].clear();
    auto deliveryRecords =
            Dao::DeliveryDao(sessionData_->dbSession_).loadDelivery(msgId);
    for (auto &record : deliveryRecords)
    {
        delivery_[msgId].emplace_back(
                    nn_make_shared<DeliveryImpl>(std::move(record)));
    }
    bool deliveryChanged = !ApiImpl::DeliveryImpl::deliveryListsEqual(oldDeliveryList, delivery_[msgId]);

    if (stateChanged)
    {
        return {Api::Event(Api::EventType::MessageStateChanged,
                           dao->conversationId(), msgId, -1)};
    }
    else if (deliveryChanged)
    {
        return {Api::Event(Api::EventType::MessageDeliveryChanged,
                           dao->conversationId(), msgId, -1)};
    }
    else
    {
        return {{}};
    }
}

Event::ApiEvents MessagesImpl::conversationWillBeRemoved(int64_t convId)
{
    Event::ApiEvents result;

    for (auto msgId : allForConversation(convId))
    {
        auto removedDao = removeFromCache(msgId);
        if (removedDao)
        {
            auto conversationId = removedDao->conversationId();
            auto events = removeFromDb(*removedDao);
            result.insert(events.begin(), events.end());

            sessionListener_->internalEvent(
                        nn_make_shared<Event::MessageRemovedEvent>(
                            conversationId, msgId));
        }
    }
    return result;
}

Event::ApiEvents MessagesImpl::messageRemoved(int64_t convId, int64_t msgId)
{
    K_UNUSED(convId);

    Event::ApiEvents result;

    auto removedDao = removeFromCache(msgId);
    if (removedDao)
    {
        result.emplace(
                    Api::EventType::MessageRemoved,
                    removedDao->conversationId(), msgId, -1);
    }
    return result;
}

bool MessagesImpl::setState(MessagesImpl::MessageState state, int64_t msgId, bool value)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto daoStateType = (state == MessageState::Read)
            ? Dao::MessageState::Read
            : Dao::MessageState::Done;

    auto iter = messages_.find(msgId);
    if (iter != messages_.end())
    {
        auto &dao = iter->second;
        if (dao.state(daoStateType) != value)
        {
            id_type conversationId = dao.conversationId();

            SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
            dao.setState(daoStateType,value);
            dao.save(Dao::CreateOld::Yes);
            tx.commit();

            sessionListener_->internalEvent(
                        nn_make_shared<Event::ConversationModifiedEvent>(
                            conversationId));
            sessionListener_->internalEvent(
                        nn_make_shared<Event::SendApiEventsEvent>(
                            Event::ApiEvents{
                                Api::Event(Api::EventType::MessageStateChanged,
                                conversationId, msgId, -1)
                            }));
            return true;
        } else {
            return false;
        }
    } else {
        // not found = nothing changed
        return false;
    }
}

}
}
