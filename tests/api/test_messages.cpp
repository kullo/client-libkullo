/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <boost/optional/optional_io.hpp>

#include <kulloclient/api/Address.h>
#include <kulloclient/api/DateTime.h>
#include <kulloclient/api/Delivery.h>
#include <kulloclient/api/Messages.h>
#include <kulloclient/api_impl/debug.h>
#include <kulloclient/api_impl/deliveryimpl.h>
#include <kulloclient/api_impl/messagesimpl.h>
#include <kulloclient/dao/conversationdao.h>
#include <kulloclient/dao/deliverydao.h>
#include <kulloclient/dao/messagedao.h>
#include <kulloclient/event/messageremovedevent.h>

using namespace testing;
using namespace Kullo;

class ApiMessages : public ApiModelTest
{
public:
    ApiMessages()
        : deliveryData(
              Util::KulloAddress("recipient#example.com"),
              Util::Delivery::failed)
    {
        deliveryData.reason = Util::Delivery::doesnt_exist;
        deliveryData.date = Util::DateTime(2015, 6, 23, 16, 06, 23, 120);

        // – All messages (id 23, 24, 25) from one sender
        // – Sent times 2015-06-23, 2015-06-25, 2015-06-24 (i.e. id 24 is latest)
        data1_.id = 23;
        data1_.read = true;
        data1_.done = true;
        data1_.address = "recipient#example.com";
        data1_.dateSent = Api::DateTime(2015, 6, 23, 16, 06, 00, 120);
        data1_.dateReceived = Api::DateTime(*deliveryData.date);
        data1_.text = "Hello, world.";
        data1_.footer = "Bye, world.";

        data2_.id = 24;
        data2_.read = true;
        data2_.done = true;
        data2_.address = "recipient#example.com";
        data2_.dateSent     = Api::DateTime(2015, 6, 24, 16, 06, 00, 120);
        data2_.dateReceived = Api::DateTime(2015, 6, 24, 16, 06, 02, 120);
        data2_.text = "Are you okay?";
        data2_.footer = "It's me again";

        data3_.id = 25;
        data3_.read = true;
        data3_.done = true;
        data3_.address = "recipient#example.com";
        data3_.dateSent     = Api::DateTime(2015, 6, 25, 16, 06, 00, 120);
        data3_.dateReceived = Api::DateTime(2015, 6, 25, 16, 06, 02, 120);
        data3_.text = "Are you okay?";
        data3_.footer = "It's me again";

        dbSession_ = Db::makeSession(sessionConfig_);
        Db::migrate(dbSession_);

        Dao::ConversationDao convDao(dbSession_);
        convDao.setParticipants(data1_.address);
        convDao.save();
        data1_.convId = convDao.id();
        data2_.convId = convDao.id();
        data3_.convId = convDao.id();

        for (const auto &d : std::vector<SampleData>{data1_, data2_, data3_})
        {
            Dao::MessageDao dao(dbSession_);
            dao.setConversationId(d.convId);
            dao.setId(d.id);
            dao.setSender(d.address);
            dao.setState(Dao::MessageState::Read, d.read);
            dao.setState(Dao::MessageState::Done, d.done);
            dao.setDateSent(d.dateSent->toString());
            dao.setDateReceived(d.dateReceived->toString());
            dao.setText(d.text);
            dao.setFooter(d.footer);
            dao.save(Dao::CreateOld::No);
        }

        Dao::DeliveryDao(dbSession_).insertDelivery(data1_.id, {deliveryData});

        makeSession();
        uut = session_->messages();
    }

protected:
    struct SampleData
    {
        int64_t convId;
        int64_t id;
        bool read;
        bool done;
        std::string address;
        boost::optional<Api::DateTime> dateSent;
        boost::optional<Api::DateTime> dateReceived;
        std::string text;
        std::string footer;
    };
    SampleData data1_, data2_, data3_;

    Util::Delivery deliveryData;

    Db::SharedSessionPtr dbSession_;
    std::shared_ptr<Api::Messages> uut;
};

K_TEST_F(ApiMessages, allForConversationWorks)
{
    auto messages = uut->allForConversation(data1_.convId);
    ASSERT_THAT(messages.size(), Eq(3u));
    EXPECT_THAT(messages[0], Eq(data1_.id));
    EXPECT_THAT(messages[1], Eq(data2_.id));
    EXPECT_THAT(messages[2], Eq(data3_.id));

    EXPECT_THAT(uut->allForConversation(42), IsEmpty());
}

K_TEST_F(ApiMessages, latestForSenderWorks)
{
    EXPECT_THAT(uut->latestForSender(Api::Address::create(data1_.address)),
                Eq(data3_.id));

    EXPECT_THAT(uut->latestForSender(Api::Address::create("noone#example.com")),
                Eq(-1));
}

K_TEST_F(ApiMessages, removeWorks)
{
    ASSERT_THAT(uut->text(data1_.id), Eq(data1_.text));

    uut->remove(data1_.id);

    // Deleted in database
    auto dao = Dao::MessageDao::load(data1_.id, Dao::Old::No, dbSession_);
    ASSERT_THAT(dao, Not(IsNull()));
    EXPECT_THAT(dao->deleted(), Eq(true));

    // Deleted in memory
    EXPECT_THAT(uut->text(data1_.id), IsEmpty());
}

K_TEST_F(ApiMessages, removeEmitsEvents)
{
    auto type = std::type_index(typeid(Event::MessageRemovedEvent));
    EXPECT_THAT(sessionListener_->internalEventCount(type), Eq(0));

    uut->remove(data1_.id);

    // a MessageRemoved API event must be emitted
    auto events = sessionListener_->externalEvents();
    EXPECT_THAT(events,
                Contains(
                    Api::Event(
                        Api::EventType::MessageRemoved,
                        data1_.convId, data1_.id, -1)
                    )
                );

    EXPECT_THAT(sessionListener_->internalEventCount(type), Eq(1));
}

K_TEST_F(ApiMessages, conversationWorks)
{
    EXPECT_THAT(uut->conversation(data1_.id), Eq(data1_.convId));
    EXPECT_THAT(uut->conversation(data2_.id), Eq(data2_.convId));
    EXPECT_THAT(uut->conversation(data3_.id), Eq(data3_.convId));

    EXPECT_THAT(uut->conversation(999), Eq(-1));
}

K_TEST_F(ApiMessages, deliveryWorks)
    {
    ApiImpl::DeliveryImpl apiDeliveryData(deliveryData);

    auto delivery = uut->deliveryState(data1_.id);
    ASSERT_THAT(delivery.size(), Eq(1u));

    EXPECT_THAT(delivery[0]->recipient()
            ->isEqualTo(apiDeliveryData.recipient()), Eq(true));
    EXPECT_THAT(delivery[0]->state(), Eq(apiDeliveryData.state()));
    EXPECT_THAT(delivery[0]->reason(), Eq(apiDeliveryData.reason()));
    EXPECT_THAT(delivery[0]->date(), Eq(apiDeliveryData.date()));

    EXPECT_THAT(uut->deliveryState(42), IsEmpty());
}

K_TEST_F(ApiMessages, isReadWorks)
{
    EXPECT_THAT(uut->isRead(data1_.id), Eq(true));

    EXPECT_THAT(uut->isRead(42), Eq(false));
}

K_TEST_F(ApiMessages, setReadWorks)
{
    ASSERT_THAT(uut->isRead(data1_.id), Eq(true));

    // Set read -> unread
    {
        uut->setRead(data1_.id, false);

        // Events
        auto result = sessionListener_->externalEvents();
        Event::ApiEvents expectedResult = {
            Api::Event(Api::EventType::MessageStateChanged, data1_.convId, data1_.id, -1),
            Api::Event(Api::EventType::ConversationChanged, data1_.convId, -1, -1)
        };
        EXPECT_THAT(result, Eq(expectedResult));

        // Updated data
        EXPECT_THAT(uut->isRead(data1_.id), Eq(false));
    }

    // Set unread -> read
    {
        uut->setRead(data1_.id, true);

        // Events
        auto result = sessionListener_->externalEvents();
        Event::ApiEvents expectedResult = {
            Api::Event(Api::EventType::MessageStateChanged, data1_.convId, data1_.id, -1),
            Api::Event(Api::EventType::ConversationChanged, data1_.convId, -1, -1)
        };
        EXPECT_THAT(result, Eq(expectedResult));

        // Updated data
        EXPECT_THAT(uut->isRead(data1_.id), Eq(true));
    }
}

K_TEST_F(ApiMessages, setReadSkipsConsecutiveCalls)
{
    ASSERT_THAT(uut->isRead(data1_.id), Eq(true));

    // Set read -> read
    {
        uut->setRead(data1_.id, true);

        // No events
        auto result = sessionListener_->externalEvents();
        auto expectedResult = Event::ApiEvents{};
        EXPECT_THAT(result, Eq(expectedResult));

        // Updated data
        EXPECT_THAT(uut->isRead(data1_.id), Eq(true));
    }
}

K_TEST_F(ApiMessages, isDoneWorks)
{
    EXPECT_THAT(uut->isDone(data1_.id), Eq(true));

    EXPECT_THAT(uut->isDone(42), Eq(false));
}

K_TEST_F(ApiMessages, setDoneWorks)
{
    ASSERT_THAT(uut->isDone(data1_.id), Eq(true));

    // Set done -> undone
    {
        uut->setDone(data1_.id, false);

        // Events
        auto result = sessionListener_->externalEvents();
        Event::ApiEvents expectedResult = {
            Api::Event(Api::EventType::MessageStateChanged, data1_.convId, data1_.id, -1),
            Api::Event(Api::EventType::ConversationChanged, data1_.convId, -1, -1)
        };
        EXPECT_THAT(result, Eq(expectedResult));

        // Updated data
        EXPECT_THAT(uut->isDone(data1_.id), Eq(false));
    }

    // Set undone -> done
    {
        uut->setDone(data1_.id, true);

        // Events
        auto result = sessionListener_->externalEvents();
        Event::ApiEvents expectedResult = {
            Api::Event(Api::EventType::MessageStateChanged, data1_.convId, data1_.id, -1),
            Api::Event(Api::EventType::ConversationChanged, data1_.convId, -1, -1)
        };
        EXPECT_THAT(result, Eq(expectedResult));

        // Updated data
        EXPECT_THAT(uut->isDone(data1_.id), Eq(true));
    }
}

K_TEST_F(ApiMessages, setDoneSkipsConsecutiveCalls)
{
    ASSERT_THAT(uut->isDone(data1_.id), Eq(true));

    // Set done -> done
    {
        uut->setDone(data1_.id, true);

        // No events
        auto result = sessionListener_->externalEvents();
        auto expectedResult = Event::ApiEvents{};
        EXPECT_THAT(result, Eq(expectedResult));

        // Updated data
        EXPECT_THAT(uut->isRead(data1_.id), Eq(true));
    }
}

K_TEST_F(ApiMessages, dateSentWorks)
{
    EXPECT_THAT(uut->dateSent(data1_.id), Eq(data1_.dateSent));

    EXPECT_THAT(uut->dateSent(42), Not(Eq(data1_.dateSent)));
}

K_TEST_F(ApiMessages, dateReceivedWorks)
{
    EXPECT_THAT(uut->dateReceived(data1_.id), Eq(data1_.dateReceived));

    EXPECT_THAT(uut->dateReceived(42), Not(Eq(data1_.dateReceived)));
}

K_TEST_F(ApiMessages, textWorks)
{
    EXPECT_THAT(uut->text(data1_.id), Eq(data1_.text));

    EXPECT_THAT(uut->text(42), Eq(""));
}

K_TEST_F(ApiMessages, footerWorks)
{
    EXPECT_THAT(uut->footer(data1_.id), Eq(data1_.footer));

    EXPECT_THAT(uut->footer(42), Eq(""));
}

K_TEST_F(ApiMessages, messageRemovedWorks)
{
    // delete message from DB
    auto dao = Dao::MessageDao::load(data1_.id, Dao::Old::No, dbSession_);
    ASSERT_THAT(dao, Not(IsNull()));
    dao->clearData();
    dao->setDeleted(true);
    dao->save(Dao::CreateOld::Yes);

    // make sure that message data is still cached in UUT
    ASSERT_THAT(uut->text(data1_.id), Not(IsEmpty()));

    // send messageRemoved event
    auto uutImpl = std::dynamic_pointer_cast<ApiImpl::MessagesImpl>(uut);
    ASSERT_THAT(uutImpl, Not(IsNull()));
    auto events = uutImpl->messageRemoved(data1_.convId, data1_.id);

    // we'd like to get a notification that the message has been deleted
    EXPECT_THAT(events,
                Contains(
                    Api::Event(
                        Api::EventType::MessageRemoved,
                        data1_.convId, data1_.id, -1)
                    )
                );

    // make sure that messageRemoved() updated the UUT
    EXPECT_THAT(uut->text(data1_.id), IsEmpty());
}

K_TEST_F(ApiMessages, idRangeWorks)
{
    for (auto id : TEST_IDS_VALID)
    {
        // id is a conversation id
        EXPECT_NO_THROW(uut->allForConversation(id));
        // id is a message id
        EXPECT_NO_THROW(uut->conversation(id));
        EXPECT_NO_THROW(uut->deliveryState(id));
        EXPECT_NO_THROW(uut->isRead(id));
        EXPECT_NO_THROW(uut->setRead(id, true));
        EXPECT_NO_THROW(uut->isDone(id));
        EXPECT_NO_THROW(uut->setDone(id, true));
        EXPECT_NO_THROW(uut->dateSent(id));
        EXPECT_NO_THROW(uut->dateReceived(id));
        EXPECT_NO_THROW(uut->text(id));
        EXPECT_NO_THROW(uut->textAsHtml(id));
        EXPECT_NO_THROW(uut->footer(id));
        EXPECT_NO_THROW(uut->remove(id));
    }

    for (auto id : TEST_IDS_INVALID)
    {
        // id is a conversation id
        EXPECT_ANY_THROW(uut->allForConversation(id));
        // id is a message id
        EXPECT_ANY_THROW(uut->deliveryState(id));
        EXPECT_ANY_THROW(uut->isRead(id));
        EXPECT_ANY_THROW(uut->setRead(id, true));
        EXPECT_ANY_THROW(uut->isDone(id));
        EXPECT_ANY_THROW(uut->setDone(id, true));
        EXPECT_ANY_THROW(uut->dateSent(id));
        EXPECT_ANY_THROW(uut->dateReceived(id));
        EXPECT_ANY_THROW(uut->text(id));
        EXPECT_ANY_THROW(uut->textAsHtml(id));
        EXPECT_ANY_THROW(uut->footer(id));
        EXPECT_ANY_THROW(uut->remove(id));
    }
}
