/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <kulloclient/api/Conversations.h>
#include <kulloclient/api/Messages.h>
#include <kulloclient/api_impl/Address.h>
#include "kulloclient/api_impl/DateTime.h"
#include <kulloclient/api_impl/conversationsimpl.h>
#include <kulloclient/api_impl/event.h>
#include <kulloclient/dao/conversationdao.h>
#include <kulloclient/event/conversationaddedevent.h>
#include <kulloclient/event/conversationmodifiedevent.h>
#include <kulloclient/event/conversationwillberemovedevent.h>

using namespace testing;
using namespace Kullo;

K_TEST(ApiConversationsStatic, emptyConversationTimestampIsInTheFuture)
{
    EXPECT_THAT(Api::DateTime::nowUtc(),
                Lt(Api::Conversations::emptyConversationTimestamp()));
}

class ApiConversations : public ApiModelTest
{
public:
    ApiConversations()
    {
        // emptyData_.id is autogenerated by and read from DB
        emptyData_.participant1 = "empty1#example.com";
        emptyData_.participant2 = "empty2#example.com";
        auto emptyParticipants =
                emptyData_.participant1 + "," + emptyData_.participant2;

        // data_.id is autogenerated by and read from DB
        const auto addressStringMe      = "me#example.com";
        const auto addressStringYou     = "you#example.com";
        const auto addressStringSomeone = "someone#example.com";

        data_.participant1 = addressStringYou;
        data_.participant2 = addressStringSomeone;
        auto participants = data_.participant2 + "," + data_.participant1; // ensure alphabetic order

        messageData1_.id = 0;
        messageData1_.sender = addressStringMe;
        messageData1_.dateSent = Api::DateTime(2014, 6, 23, 16, 06, 00, 120);
        messageData1_.dateReceived = Api::DateTime(2014, 6, 23, 16, 06, 01, 120);
        messageData1_.read = true;
        messageData1_.done = true;

        messageData2_.id = 22;
        messageData2_.sender = addressStringYou;
        messageData2_.dateSent = Api::DateTime(2015, 6, 23, 16, 06, 00, 120);
        messageData2_.dateReceived = Api::DateTime(2015, 6, 23, 16, 06, 01, 120);
        messageData2_.read = true;
        messageData2_.done = false;

        messageData3_.id = 23;
        messageData3_.sender = addressStringSomeone;
        messageData3_.dateSent = Api::DateTime(2016, 6, 23, 16, 06, 00, 120);
        messageData3_.dateReceived = Api::DateTime(2016, 6, 23, 16, 06, 01, 120);
        messageData3_.read = false;
        messageData3_.done = false;

        dbSession_ = Db::makeSession(sessionConfig_);
        Db::migrate(dbSession_);

        Dao::ConversationDao emptyDao(dbSession_);
        emptyDao.setParticipants(emptyParticipants);
        emptyDao.save();
        emptyData_.id = emptyDao.id();

        Dao::ConversationDao dao(dbSession_);
        dao.setParticipants(participants);
        dao.save();
        data_.id = dao.id();

        {
            // outgoing
            Dao::MessageDao dao(dbSession_);
            dao.setSender(messageData1_.sender);
            dao.setConversationId(data_.id);
            dao.setId(messageData1_.id);
            dao.setDateSent(messageData1_.dateSent->toString());
            dao.setDateReceived(messageData1_.dateReceived->toString());
            dao.setState(Dao::MessageState::Read, messageData1_.read);
            dao.setState(Dao::MessageState::Done, messageData1_.done);
            dao.save(Dao::CreateOld::No);
        }

        {
            // incoming 1
            Dao::MessageDao dao(dbSession_);
            dao.setSender(messageData2_.sender);
            dao.setConversationId(data_.id);
            dao.setId(messageData2_.id);
            dao.setDateSent(messageData2_.dateSent->toString());
            dao.setDateReceived(messageData2_.dateReceived->toString());
            dao.setState(Dao::MessageState::Read, messageData2_.read);
            dao.setState(Dao::MessageState::Done, messageData2_.done);
            dao.save(Dao::CreateOld::No);
        }

        {
            // incoming 2
            Dao::MessageDao dao(dbSession_);
            dao.setSender(messageData3_.sender);
            dao.setConversationId(data_.id);
            dao.setId(messageData3_.id);
            dao.setDateSent(messageData3_.dateSent->toString());
            dao.setDateReceived(messageData3_.dateReceived->toString());
            dao.setState(Dao::MessageState::Read, messageData3_.read);
            dao.setState(Dao::MessageState::Done, messageData3_.done);
            dao.save(Dao::CreateOld::No);
        }

        makeSession();
        uut_ = session_->conversations().as_nullable();
    }

protected:
    struct
    {
        int64_t id;
        std::string participant1;
        std::string participant2;
    } emptyData_;

    struct
    {
        int64_t id;
        std::string participant1;
        std::string participant2;
    } data_;

    struct MessageData
    {
        int64_t id;
        std::string sender;
        boost::optional<Api::DateTime> dateSent;
        boost::optional<Api::DateTime> dateReceived;
        bool read;
        bool done;
    };
    MessageData messageData1_;
    MessageData messageData2_;
    MessageData messageData3_;

    Db::SharedSessionPtr dbSession_;
    std::shared_ptr<Api::Conversations> uut_;
};

K_TEST_F(ApiConversations, allWorks)
{
    auto conversations = uut_->all();
    EXPECT_THAT(conversations, WhenSorted(ElementsAre(1, 2)));
}

K_TEST_F(ApiConversations, getWorks)
{
    std::unordered_set<Api::Address> participants;
    participants.emplace(Api::Address(data_.participant1));
    participants.emplace(Api::Address(data_.participant2));
    EXPECT_THAT(uut_->get(participants), Eq(data_.id));

    participants.emplace(Api::Address("nosuchuser#example.com"));
    EXPECT_THAT(uut_->get(participants), Eq(-1));
}

K_TEST_F(ApiConversations, getWorksWithDuplicateAddresses)
{
    std::unordered_set<Api::Address> participants;
    participants.emplace(Api::Address(data_.participant1));
    participants.emplace(Api::Address(data_.participant1));
    participants.emplace(Api::Address(data_.participant2));
    EXPECT_THAT(uut_->get(participants), Eq(data_.id));

    participants.emplace(Api::Address("nosuchuser#example.com"));
    EXPECT_THAT(uut_->get(participants), Eq(-1));
}

K_TEST_F(ApiConversations, addReturnsExistingConversation)
{
    std::unordered_set<Api::Address> participants;
    participants.emplace(Api::Address(data_.participant1));
    participants.emplace(Api::Address(data_.participant2));

    EXPECT_THAT(uut_->add(participants), Eq(data_.id));
}

K_TEST_F(ApiConversations, addWorksWithDuplicateAddresses)
{
    std::unordered_set<Api::Address> participants;
    participants.emplace(Api::Address(data_.participant1));
    participants.emplace(Api::Address(data_.participant1));
    participants.emplace(Api::Address(data_.participant2));
    auto convId = uut_->add(participants);

    // must not create new conversation but return existing with {part1, part2}
    EXPECT_THAT(convId, Eq(data_.id));
}

K_TEST_F(ApiConversations, addReturnsNewConversation)
{
    std::unordered_set<Api::Address> participants;
    participants.emplace(Api::Address(data_.participant1));
    participants.emplace(Api::Address(data_.participant2));
    participants.emplace(Api::Address("nosuchuser#example.com"));
    auto convId = uut_->add(participants);

    EXPECT_THAT(convId, Ge(0));
    EXPECT_THAT(convId, Ne(data_.id));
}

K_TEST_F(ApiConversations, addStoresToMemoryAndDatabase)
{
    std::unordered_set<Api::Address> participants;
    participants.emplace(Api::Address("local-new-guy1#example.com"));
    participants.emplace(Api::Address("local-new-guy2#example.com"));

    auto convId = uut_->add(participants);

    // non-existing conversation defaults to empty participants set
    EXPECT_THAT(uut_->participants(convId).size(), Eq(2u));

    // Existing in database
    {
        auto dao = Dao::ConversationDao::load(convId, dbSession_);
        ASSERT_THAT(dao, Not(IsNull()));
        EXPECT_THAT(dao->participantsList().size(), Eq(2u));
    }
}

K_TEST_F(ApiConversations, addEmitsEvent)
{
    std::unordered_set<Api::Address> participants;
    participants.emplace(Api::Address("local-new-guy1#example.com"));
    participants.emplace(Api::Address("local-new-guy2#example.com"));

    auto addedConvId = uut_->add(participants);

    auto type = std::type_index(typeid(Event::ConversationAddedEvent));
    EXPECT_THAT(sessionListener_->internalEventCount(type), Eq(1));

    // send conversationAdded event
    auto uutImpl = std::dynamic_pointer_cast<ApiImpl::ConversationsImpl>(uut_);
    ASSERT_THAT(uutImpl, Not(IsNull()));
    auto events = uutImpl->conversationAdded(addedConvId);

    // we'd like to get a notification that the conversation has been removed
    EXPECT_THAT(events, Contains(
                    Api::Event(Api::EventType::ConversationAdded, addedConvId, -1, -1))
                );

    // make sure that conversationAdded() updated the UUT
    EXPECT_THAT(uut_->participants(addedConvId).size(), Eq(2u));
}

K_TEST_F(ApiConversations, addDoesntAcceptCurrentUser)
{
    const auto currentUserAddressString = address_.toString();

    std::unordered_set<Api::Address> participants1;
    participants1.emplace(Api::Address(currentUserAddressString));

    std::unordered_set<Api::Address> participants2;
    participants2.emplace(Api::Address("local-new-guy1#example.com"));
    participants2.emplace(Api::Address(currentUserAddressString));

    std::unordered_set<Api::Address> participants3;
    participants3.emplace(Api::Address("local-new-guy1#example.com"));
    participants3.emplace(Api::Address(currentUserAddressString));
    participants3.emplace(Api::Address("local-new-guy2#example.com"));

    EXPECT_THROW(uut_->add(participants1), Util::AssertionFailed);
    EXPECT_THROW(uut_->add(participants2), Util::AssertionFailed);
    EXPECT_THROW(uut_->add(participants3), Util::AssertionFailed);
}

K_TEST_F(ApiConversations, triggerRemovalWorks)
{
    // Existing in memory
    ASSERT_THAT(uut_->participants(data_.id).size(), Gt(0u));

    // Existing in database
    auto dao1 = Dao::ConversationDao::load(data_.id, dbSession_);
    EXPECT_THAT(dao1, Not(IsNull()));

    uut_->triggerRemoval(data_.id);

    // Deleted in memory
    EXPECT_THAT(uut_->participants(data_.id).size(), Eq(0u));

    // Deleted in database
    auto dao2 = Dao::ConversationDao::load(data_.id, dbSession_);
    EXPECT_THAT(dao2, IsNull());
}

K_TEST_F(ApiConversations, triggerRemovalEmitsEvents)
{
    auto type = std::type_index(typeid(Event::ConversationWillBeRemovedEvent));
    EXPECT_THAT(sessionListener_->internalEventCount(type), Eq(0));

    uut_->triggerRemoval(data_.id);

    // a ConversationRemoved API event must be emitted
    auto events = sessionListener_->externalEvents();
    EXPECT_THAT(events, Contains(
                    Api::Event(Api::EventType::ConversationRemoved, data_.id, -1, -1))
                );

    EXPECT_THAT(sessionListener_->internalEventCount(type), Eq(1));
}

K_TEST_F(ApiConversations, participantsWorks)
{
    auto participants = uut_->participants(data_.id);
    ASSERT_THAT(participants.size(), Eq(2u));
    bool found1 = false, found2 = false;
    for (auto &part : participants)
    {
        found1 = found1 || (part.toString() == data_.participant1);
        found2 = found2 || (part.toString() == data_.participant2);
    }
    EXPECT_THAT(found1, Eq(true));
    EXPECT_THAT(found2, Eq(true));

    EXPECT_THAT(uut_->participants(42), IsEmpty());
}

K_TEST_F(ApiConversations, changedEventFromMessageState)
{
    // Set to undone
    session_->messages()->setDone(messageData1_.id, false);
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationChanged);
        EXPECT_THAT(events, Contains(
                        Api::Event(Api::EventType::ConversationChanged, data_.id, -1, -1))
                    );
    }

    sessionListener_->resetEvents();

    // Set to undone again
    session_->messages()->setDone(messageData1_.id, false);
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationChanged);
        EXPECT_THAT(events, IsEmpty());
    }

    sessionListener_->resetEvents();

    // Set to done
    session_->messages()->setDone(messageData1_.id, true);
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationChanged);
        EXPECT_THAT(events, Contains(
                        Api::Event(Api::EventType::ConversationChanged, data_.id, -1, -1))
                    );
    }

    sessionListener_->resetEvents();

    // Set to done again
    session_->messages()->setDone(messageData1_.id, true);
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationChanged);
        EXPECT_THAT(events, IsEmpty());
    }

    sessionListener_->resetEvents();

    // Set to unread
    session_->messages()->setRead(messageData1_.id, false);
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationChanged);
        EXPECT_THAT(events, Contains(
                        Api::Event(Api::EventType::ConversationChanged, data_.id, -1, -1))
                    );
    }

    sessionListener_->resetEvents();

    // Set to unread again
    session_->messages()->setRead(messageData1_.id, false);
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationChanged);
        EXPECT_THAT(events, IsEmpty());
    }

    sessionListener_->resetEvents();

    // Set to read
    session_->messages()->setRead(messageData1_.id, true);
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationChanged);
        EXPECT_THAT(events, Contains(
                        Api::Event(Api::EventType::ConversationChanged, data_.id, -1, -1))
                    );
    }

    sessionListener_->resetEvents();

    // Set to read again
    session_->messages()->setRead(messageData1_.id, true);
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationChanged);
        EXPECT_THAT(events, IsEmpty());
    }
}

K_TEST_F(ApiConversations, latestMessageTimestampChangedEventFromDeleting)
{
    // Delete oldest
    session_->messages()->remove(messageData1_.id);
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationLatestMessageTimestampChanged);
        EXPECT_THAT(events, IsEmpty());
    }

    sessionListener_->resetEvents();

    // Delete newest
    session_->messages()->remove(messageData3_.id);
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationLatestMessageTimestampChanged);
        EXPECT_THAT(events, Contains(
                        Api::Event(Api::EventType::ConversationLatestMessageTimestampChanged, data_.id, -1, -1))
                    );
    }
}

K_TEST_F(ApiConversations, latestMessageTimestampChangedEventFromAdding)
{
    // Add old message
    {
        Dao::MessageDao dao(dbSession_);
        dao.setSender(messageData1_.sender);
        dao.setConversationId(data_.id);
        dao.setId(2);
        dao.setDateSent(Api::DateTime(2014, 7, 23, 16, 06, 00, 120).toString());
        dao.setDateReceived(Api::DateTime(2014, 7, 23, 16, 06, 00, 120).toString());
        dao.save(Dao::CreateOld::No);
        sessionListener_->internalEvent(Kullo::nn_make_shared<Event::ConversationModifiedEvent>(data_.id));
    }

    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationLatestMessageTimestampChanged);
        EXPECT_THAT(events, IsEmpty());
    }

    sessionListener_->resetEvents();

    // Add new message
    {
        Dao::MessageDao dao(dbSession_);
        dao.setSender(messageData1_.sender);
        dao.setConversationId(data_.id);
        dao.setId(2222);
        dao.setDateSent(Api::DateTime(2018, 7, 23, 16, 06, 00, 120).toString());
        dao.setDateReceived(Api::DateTime(2018, 7, 23, 16, 06, 00, 120).toString());
        dao.save(Dao::CreateOld::No);
        sessionListener_->internalEvent(Kullo::nn_make_shared<Event::ConversationModifiedEvent>(data_.id));
    }
    {
        auto events = sessionListener_->externalEvents(Api::EventType::ConversationLatestMessageTimestampChanged);
        EXPECT_THAT(events, Contains(
                        Api::Event(Api::EventType::ConversationLatestMessageTimestampChanged, data_.id, -1, -1))
                    );
    }
}

K_TEST_F(ApiConversations, totalMessagesWorks)
{
    EXPECT_THAT(uut_->totalMessages(emptyData_.id), Eq(0));
    EXPECT_THAT(uut_->totalMessages(data_.id), Eq(3));

    EXPECT_THAT(uut_->totalMessages(42), Eq(0));
}

K_TEST_F(ApiConversations, unreadMessagesWorks)
{
    EXPECT_THAT(uut_->unreadMessages(emptyData_.id), Eq(0));
    EXPECT_THAT(uut_->unreadMessages(data_.id), Eq(1));

    EXPECT_THAT(uut_->unreadMessages(42), Eq(0));
}

K_TEST_F(ApiConversations, undoneMessagesWorks)
{
    EXPECT_THAT(uut_->undoneMessages(emptyData_.id), Eq(0));
    EXPECT_THAT(uut_->undoneMessages(data_.id), Eq(2));

    EXPECT_THAT(uut_->undoneMessages(42), Eq(0));
}

K_TEST_F(ApiConversations, incomingMessagesWorks)
{
    EXPECT_THAT(uut_->incomingMessages(emptyData_.id), Eq(0));
    EXPECT_THAT(uut_->incomingMessages(data_.id), Eq(2));

    EXPECT_THAT(uut_->incomingMessages(42), Eq(0));
}

K_TEST_F(ApiConversations, outgoingMessagesWorks)
{
    EXPECT_THAT(uut_->outgoingMessages(emptyData_.id), Eq(0));
    EXPECT_THAT(uut_->outgoingMessages(data_.id), Eq(1));

    EXPECT_THAT(uut_->outgoingMessages(42), Eq(0));
}

K_TEST_F(ApiConversations, latestMessageTimestampWorks)
{
    EXPECT_THAT(uut_->latestMessageTimestamp(emptyData_.id),
                Eq(Api::Conversations::emptyConversationTimestamp()));

    EXPECT_THAT(uut_->latestMessageTimestamp(data_.id),
                Eq(messageData3_.dateReceived));

    EXPECT_THAT(uut_->latestMessageTimestamp(42),
                Not(Eq(messageData1_.dateReceived)));
    EXPECT_THAT(uut_->latestMessageTimestamp(42),
                Not(Eq(messageData2_.dateReceived)));
    EXPECT_THAT(uut_->latestMessageTimestamp(42),
                Not(Eq(messageData3_.dateReceived)));
}

K_TEST_F(ApiConversations, idRangeWorks)
{
    for (auto convId : TEST_IDS_VALID)
    {
        EXPECT_NO_THROW(uut_->participants(convId));
        EXPECT_NO_THROW(uut_->totalMessages(convId));
        EXPECT_NO_THROW(uut_->unreadMessages(convId));
        EXPECT_NO_THROW(uut_->undoneMessages(convId));
        EXPECT_NO_THROW(uut_->incomingMessages(convId));
        EXPECT_NO_THROW(uut_->outgoingMessages(convId));
        EXPECT_NO_THROW(uut_->latestMessageTimestamp(convId));
        EXPECT_NO_THROW(uut_->triggerRemoval(convId));
    }

    for (auto convId : TEST_IDS_INVALID)
    {
        EXPECT_ANY_THROW(uut_->participants(convId));
        EXPECT_ANY_THROW(uut_->totalMessages(convId));
        EXPECT_ANY_THROW(uut_->unreadMessages(convId));
        EXPECT_ANY_THROW(uut_->undoneMessages(convId));
        EXPECT_ANY_THROW(uut_->incomingMessages(convId));
        EXPECT_ANY_THROW(uut_->outgoingMessages(convId));
        EXPECT_ANY_THROW(uut_->latestMessageTimestamp(convId));
        EXPECT_ANY_THROW(uut_->triggerRemoval(convId));
    }
}
