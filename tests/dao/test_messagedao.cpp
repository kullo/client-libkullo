/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/dao/messagedao.h>

#include "tests/dbtest.h"

using namespace testing;
using namespace Kullo;

namespace {
    struct TestData
    {
        const id_type id = 42;
        const id_type conversationId = 23;
        const lastModified_type lastModified = 123456789;
        const std::string sender = "test#kullo.net";
        const bool deleted = true;
        const std::string dateSent = "2001-01-20T13:37:42Z";
        const std::string dateReceived = "2525-02-25T23:42:05Z";
        const std::string text = "Hello, world.";
        const std::string footer = "Goodbye, world.";
        const std::vector<unsigned char> symmetricKey = std::initializer_list<unsigned char>({23, 42, 5});
        const bool read = true;
        const bool done = true;
    };
}

class MessageDao : public DbTest
{
};

K_TEST_F(MessageDao, insertsNewMessage)
{
    Dao::MessageDao dao(session_);
    dao.setId(123);
    ASSERT_THAT(dao.save(Dao::CreateOld::No), Eq(true));
    ASSERT_THAT(dao.save(Dao::CreateOld::No), Eq(false));
}


K_TEST_F(MessageDao, insertsNewMessageIdRange)
{
    {
        Dao::MessageDao dao(session_);
        dao.setId(Kullo::ID_MIN);
        dao.setConversationId(Kullo::ID_MIN);
        EXPECT_THAT(dao.save(Dao::CreateOld::No), Eq(true));
    }
    {
        Dao::MessageDao dao(session_);
        dao.setId(Kullo::ID_MAX);
        dao.setConversationId(Kullo::ID_MAX);
        EXPECT_THAT(dao.save(Dao::CreateOld::No), Eq(true));
    }
}

K_TEST_F(MessageDao, doesntModifySetData)
{
    TestData data;
    Dao::MessageDao dao(session_);

    dao.setId(data.id);
    dao.setConversationId(data.conversationId);
    dao.setLastModified(data.lastModified);
    dao.setSender(data.sender);
    dao.setDeleted(data.deleted);
    dao.setDateSent(data.dateSent);
    dao.setDateReceived(data.dateReceived);
    dao.setText(data.text);
    dao.setFooter(data.footer);
    dao.setSymmetricKey(data.symmetricKey);
    dao.setState(Dao::MessageState::Read, data.read);
    dao.setState(Dao::MessageState::Done, data.done);

    EXPECT_THAT(dao.id(), Eq(data.id));
    EXPECT_THAT(dao.conversationId(), Eq(data.conversationId));
    EXPECT_THAT(dao.lastModified(), Eq(data.lastModified));
    EXPECT_THAT(dao.sender(), Eq(data.sender));
    EXPECT_THAT(dao.deleted(), Eq(data.deleted));
    EXPECT_THAT(dao.dateSent(), Eq(data.dateSent));
    EXPECT_THAT(dao.dateReceived(), Eq(data.dateReceived));
    EXPECT_THAT(dao.text(), Eq(data.text));
    EXPECT_THAT(dao.footer(), Eq(data.footer));
    EXPECT_THAT(dao.symmetricKey(), Eq(data.symmetricKey));
    EXPECT_THAT(dao.state(Dao::MessageState::Read), Eq(data.read));
    EXPECT_THAT(dao.state(Dao::MessageState::Done), Eq(data.done));
    EXPECT_THAT(dao.state(Dao::MessageState::Unread), Eq(!data.read));
    EXPECT_THAT(dao.state(Dao::MessageState::Undone), Eq(!data.done));
}

K_TEST_F(MessageDao, loadsSavedData)
{
    TestData data;
    Dao::MessageDao dao(session_);

    dao.setId(data.id);
    dao.setConversationId(data.conversationId);
    dao.setLastModified(data.lastModified);
    dao.setSender(data.sender);
    dao.setDeleted(data.deleted);
    dao.setDateSent(data.dateSent);
    dao.setDateReceived(data.dateReceived);
    dao.setText(data.text);
    dao.setFooter(data.footer);
    dao.setSymmetricKey(data.symmetricKey);
    dao.setState(Dao::MessageState::Read, data.read);
    dao.setState(Dao::MessageState::Done, data.done);
    ASSERT_THAT(dao.save(Dao::CreateOld::No), Eq(true));

    auto loaded = Dao::MessageDao::load(data.id, Dao::Old::No, session_);
    ASSERT_THAT(loaded, NotNull());
    EXPECT_THAT(loaded->id(), Eq(data.id));
    EXPECT_THAT(loaded->conversationId(), Eq(data.conversationId));
    EXPECT_THAT(loaded->lastModified(), Eq(data.lastModified));
    EXPECT_THAT(loaded->sender(), Eq(data.sender));
    EXPECT_THAT(loaded->deleted(), Eq(data.deleted));
    EXPECT_THAT(loaded->dateSent(), Eq(data.dateSent));
    EXPECT_THAT(loaded->dateReceived(), Eq(data.dateReceived));
    EXPECT_THAT(loaded->text(), Eq(data.text));
    EXPECT_THAT(loaded->footer(), Eq(data.footer));
    EXPECT_THAT(loaded->symmetricKey(), Eq(data.symmetricKey));
    EXPECT_THAT(loaded->state(Dao::MessageState::Read), Eq(data.read));
    EXPECT_THAT(loaded->state(Dao::MessageState::Done), Eq(data.done));
    EXPECT_THAT(loaded->state(Dao::MessageState::Unread), Eq(!data.read));
    EXPECT_THAT(loaded->state(Dao::MessageState::Undone), Eq(!data.done));
}

K_TEST_F(MessageDao, equals)
{
    {
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(123);
        dao2.setId(123);
        EXPECT_THAT(dao1, Eq(dao2));
    }
    {
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(144);
        dao2.setId(144);
        dao1.setDeleted(false);
        dao2.setDeleted(false);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, true);
        dao1.setState(Dao::MessageState::Done, false);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(1);
        EXPECT_THAT(dao1, Eq(dao2));
    }
}

K_TEST_F(MessageDao, unequal)
{
    {   // different id
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(123);
        dao2.setId(127);
        EXPECT_THAT(dao1, Ne(dao2));
    }
    {   // different read state
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(123);
        dao2.setId(123);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, false);
        EXPECT_THAT(dao1, Ne(dao2));
    }
    {   // different done state
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(123);
        dao2.setId(123);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, true);
        dao1.setState(Dao::MessageState::Done, false);
        dao2.setState(Dao::MessageState::Done, true);
        EXPECT_THAT(dao1, Ne(dao2));
    }
    {   // different meta version
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(123);
        dao2.setId(123);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, true);
        dao1.setState(Dao::MessageState::Done, false);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(2);
        EXPECT_THAT(dao1, Ne(dao2));
    }
    {   // different deleted value
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(123);
        dao2.setId(123);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, true);
        dao1.setState(Dao::MessageState::Done, false);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(1);
        dao1.setDeleted(false);
        dao1.setDeleted(true);
        EXPECT_THAT(dao1, Ne(dao2));
    }
}

K_TEST_F(MessageDao, equalsDeletedOverwrites)
{
    // MessageDaos are equal if both deleted but only when id matches
    {
        // both deleted, same id
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(144);
        dao2.setId(144);
        dao1.setDeleted(true);
        dao2.setDeleted(true);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, false);
        dao1.setState(Dao::MessageState::Done, true);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(2);
        EXPECT_THAT(dao1, Eq(dao2));
    }
    {
        // both deleted, different id
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(144);
        dao2.setId(145);
        dao1.setDeleted(true);
        dao2.setDeleted(true);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, false);
        dao1.setState(Dao::MessageState::Done, true);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(2);
        EXPECT_THAT(dao1, Ne(dao2));
    }
}

K_TEST_F(MessageDao, equalsWhileSomeValuesDiffer)
{
    {
        // text differs
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(144);
        dao2.setId(144);
        dao1.setDeleted(false);
        dao2.setDeleted(false);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, true);
        dao1.setState(Dao::MessageState::Done, false);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(1);
        dao1.setText("foo");
        dao2.setText("bar");
        EXPECT_THAT(dao1, Eq(dao2));
    }
    {
        // footer differs
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(144);
        dao2.setId(144);
        dao1.setDeleted(false);
        dao2.setDeleted(false);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, true);
        dao1.setState(Dao::MessageState::Done, false);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(1);
        dao1.setText("txt");
        dao2.setText("txt");
        dao1.setFooter("foo");
        dao2.setFooter("bar");
        EXPECT_THAT(dao1, Eq(dao2));
    }
    {
        // dateSent differs
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(144);
        dao2.setId(144);
        dao1.setDeleted(false);
        dao2.setDeleted(false);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, true);
        dao1.setState(Dao::MessageState::Done, false);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(1);
        dao1.setText("txt");
        dao2.setText("txt");
        dao1.setFooter("imprint");
        dao2.setFooter("imprint");
        dao1.setDateSent("2525-02-25T00:00:00Z");
        dao1.setDateSent("2525-02-25T00:00:99Z");
        EXPECT_THAT(dao1, Eq(dao2));
    }
    {
        // dateReceived differs
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(144);
        dao2.setId(144);
        dao1.setDeleted(false);
        dao2.setDeleted(false);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, true);
        dao1.setState(Dao::MessageState::Done, false);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(1);
        dao1.setText("txt");
        dao2.setText("txt");
        dao1.setFooter("imprint");
        dao2.setFooter("imprint");
        dao1.setDateSent("2525-02-24T23:59:00Z");
        dao2.setDateSent("2525-02-24T23:59:00Z");
        dao1.setDateReceived("2525-02-25T00:00:00Z");
        dao2.setDateReceived("2525-02-25T00:00:99Z");
        EXPECT_THAT(dao1, Eq(dao2));
    }
    {
        // sender differs
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(144);
        dao2.setId(144);
        dao1.setDeleted(false);
        dao2.setDeleted(false);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, true);
        dao1.setState(Dao::MessageState::Done, false);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(1);
        dao1.setText("txt");
        dao2.setText("txt");
        dao1.setFooter("imprint");
        dao2.setFooter("imprint");
        dao1.setDateSent("2525-02-24T23:59:00Z");
        dao2.setDateSent("2525-02-24T23:59:00Z");
        dao1.setDateReceived("2525-02-25T00:00:00Z");
        dao2.setDateReceived("2525-02-25T00:00:00Z");
        dao1.setSender("test1#kullo.net");
        dao2.setSender("test2#kullo.net");
        EXPECT_THAT(dao1, Eq(dao2));
    }
    {
        // conversation id differs
        Dao::MessageDao dao1(session_);
        Dao::MessageDao dao2(session_);
        dao1.setId(144);
        dao2.setId(144);
        dao1.setDeleted(false);
        dao2.setDeleted(false);
        dao1.setState(Dao::MessageState::Read, true);
        dao2.setState(Dao::MessageState::Read, true);
        dao1.setState(Dao::MessageState::Done, false);
        dao2.setState(Dao::MessageState::Done, false);
        dao1.setMetaVersion(1);
        dao2.setMetaVersion(1);
        dao1.setText("txt");
        dao2.setText("txt");
        dao1.setFooter("imprint");
        dao2.setFooter("imprint");
        dao1.setDateSent("2525-02-24T23:59:00Z");
        dao2.setDateSent("2525-02-24T23:59:00Z");
        dao1.setDateReceived("2525-02-25T00:00:00Z");
        dao2.setDateReceived("2525-02-25T00:00:00Z");
        dao1.setSender("test#kullo.net");
        dao2.setSender("test#kullo.net");
        dao1.setConversationId(1);
        dao2.setConversationId(2);
        EXPECT_THAT(dao1, Eq(dao2));
    }
}

K_TEST_F(MessageDao, oldCreatedProperly)
{
    TestData data;
    {
        // old must not exist
        {
            Dao::MessageDao dao(session_);
            dao.setId(123);
            dao.setLastModified(data.lastModified);
            dao.save(Dao::CreateOld::No);
        }

        auto local = Dao::MessageDao::load(123, Dao::Old::No, session_);
        auto old   = Dao::MessageDao::load(123, Dao::Old::Yes, session_);

        ASSERT_THAT(local, NotNull());
        ASSERT_THAT(old, IsNull());
    }
    {
        // old must exist and differ from local
        {
            Dao::MessageDao dao(session_);
            dao.setId(124);
            dao.setLastModified(10000001);
            dao.setText("initial text");
            dao.save(Dao::CreateOld::No);

            dao.setText("another text");
            dao.save(Dao::CreateOld::Yes);
        }

        auto local = Dao::MessageDao::load(124, Dao::Old::No, session_);
        auto old   = Dao::MessageDao::load(124, Dao::Old::Yes, session_);

        ASSERT_THAT(local, NotNull());
        ASSERT_THAT(old, NotNull());

        EXPECT_THAT(local->old(), Eq(false));
        EXPECT_THAT(old->old(), Eq(true));

        EXPECT_THAT(old->text(), Eq("initial text"));
        EXPECT_THAT(local->text(), Eq("another text"));
    }
}
