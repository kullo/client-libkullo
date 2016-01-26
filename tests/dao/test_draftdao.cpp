/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <kulloclient/dao/draftdao.h>

#include "tests/dbtest.h"

using namespace testing;
using namespace Kullo;

namespace {
    struct TestData
    {
        // Don't test `lastModified`, since draft is not used in sync at the moment.
        const id_type id = 42;
        const id_type conversationId = 23;
        const std::string senderName = "Mr. X";
        const std::string senderName2 = "Mr. Y";
        const std::string senderOrganization = "XY Corp.";
        const std::string senderOrganization2 = "Z Corp.";
        const std::string text = "Hello, world.";
        const std::string text2 = "Peace.";
        const std::string footer = "Goodbye, world.";
        const std::string footer2 = "Cya.";
        const Dao::DraftState state = Dao::DraftState::Editing;
    };
}

class DraftDao : public DbTest
{
};

K_TEST_F(DraftDao, insertsNewDraftMinimal)
{
    Dao::DraftDao dao(session_);
    dao.setId(123);
    ASSERT_THAT(dao.save(), Eq(true));
}

K_TEST_F(DraftDao, insertsNewDraftIdRange)
{
    {
        Dao::DraftDao dao(session_);
        dao.setId(Kullo::ID_MIN);
        dao.setConversationId(Kullo::ID_MIN);
        EXPECT_THAT(dao.save(), Eq(true));
    }
    {
        Dao::DraftDao dao(session_);
        dao.setId(Kullo::ID_MAX);
        dao.setConversationId(Kullo::ID_MAX);
        EXPECT_THAT(dao.save(), Eq(true));
    }
}

K_TEST_F(DraftDao, doesntModifySetData)
{
    TestData data;

    Dao::DraftDao dao(session_);
    dao.setId(data.id);
    dao.setConversationId(data.conversationId);
    dao.setSenderName(data.senderName);
    dao.setSenderOrganization(data.senderOrganization);
    dao.setText(data.text);
    dao.setFooter(data.footer);
    dao.setState(data.state);

    EXPECT_THAT(dao.id(), Eq(data.id));
    EXPECT_THAT(dao.conversationId(), Eq(data.conversationId));
    EXPECT_THAT(dao.senderName(), Eq(data.senderName));
    EXPECT_THAT(dao.senderOrganization(), Eq(data.senderOrganization));
    EXPECT_THAT(dao.text(), Eq(data.text));
    EXPECT_THAT(dao.footer(), Eq(data.footer));
    EXPECT_THAT(dao.state(), Eq(data.state));
}

K_TEST_F(DraftDao, loadsSavedData)
{
    TestData data;

    Dao::DraftDao dao(session_);
    dao.setId(data.id);
    dao.setConversationId(data.conversationId);
    dao.setSenderName(data.senderName);
    dao.setSenderOrganization(data.senderOrganization);
    dao.setText(data.text);
    dao.setFooter(data.footer);
    dao.setState(data.state);
    ASSERT_THAT(dao.save(), Eq(true));

    auto loaded = Dao::DraftDao::load(data.conversationId, session_);
    ASSERT_THAT(loaded, NotNull());
    EXPECT_THAT(loaded->id(), Eq(data.id));
    EXPECT_THAT(loaded->conversationId(), Eq(data.conversationId));
    EXPECT_THAT(loaded->senderName(), Eq(data.senderName));
    EXPECT_THAT(loaded->senderOrganization(), Eq(data.senderOrganization));
    EXPECT_THAT(loaded->text(), Eq(data.text));
    EXPECT_THAT(loaded->footer(), Eq(data.footer));
    EXPECT_THAT(loaded->state(), Eq(data.state));
}

K_TEST_F(DraftDao, loadAndUpdateSavedData)
{
    TestData data;

    Dao::DraftDao dao(session_);
    dao.setId(data.id);
    dao.setConversationId(data.conversationId);
    dao.setSenderName(data.senderName);
    dao.setSenderOrganization(data.senderOrganization);
    dao.setText(data.text);
    dao.setFooter(data.footer);
    dao.setState(data.state);
    ASSERT_THAT(dao.save(), Eq(true));

    auto loaded = Dao::DraftDao::load(data.conversationId, session_);
    ASSERT_THAT(loaded, NotNull());
    EXPECT_THAT(loaded->id(), Eq(data.id));
    EXPECT_THAT(loaded->conversationId(), Eq(data.conversationId));
    EXPECT_THAT(loaded->senderName(), Eq(data.senderName));
    EXPECT_THAT(loaded->senderOrganization(), Eq(data.senderOrganization));
    EXPECT_THAT(loaded->text(), Eq(data.text));
    EXPECT_THAT(loaded->footer(), Eq(data.footer));
    EXPECT_THAT(loaded->state(), Eq(data.state));

    loaded->setSenderName(data.senderName2);
    loaded->setSenderOrganization(data.senderOrganization2);
    loaded->setText(data.text2);
    loaded->setFooter(data.footer2);
    ASSERT_THAT(loaded->save(), Eq(true));

    auto loaded2 = Dao::DraftDao::load(data.conversationId, session_);
    ASSERT_THAT(loaded2, NotNull());
    EXPECT_THAT(loaded2->id(), Eq(data.id));
    EXPECT_THAT(loaded2->conversationId(), Eq(data.conversationId));
    EXPECT_THAT(loaded2->senderName(), Eq(data.senderName2));
    EXPECT_THAT(loaded2->senderOrganization(), Eq(data.senderOrganization2));
    EXPECT_THAT(loaded2->text(), Eq(data.text2));
    EXPECT_THAT(loaded2->footer(), Eq(data.footer2));
}
