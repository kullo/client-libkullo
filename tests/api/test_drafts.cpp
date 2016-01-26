/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <kulloclient/api/DraftState.h>
#include <kulloclient/api_impl/sessionimpl.h>
#include <kulloclient/dao/draftdao.h>

using namespace testing;
using namespace Kullo;

namespace {
const std::string TEST_STRING = "Hello, world";
}

class ApiDrafts : public ApiModelTest
{
public:
    ApiDrafts()
    {
        dbSession_ = Db::makeSession(dbPath_);
        Db::migrate(dbSession_);

        data.convId = 1;

        Dao::DraftDao dao(dbSession_);
        dao.setConversationId(data.convId);
        dao.setText(TEST_STRING);
        dao.save();

        makeSession();
        uut = session_->drafts();
    }

protected:
    struct
    {
        int64_t convId;
    } data;

    Db::SharedSessionPtr dbSession_;
    std::shared_ptr<Api::Drafts> uut;
};

K_TEST_F(ApiDrafts, textWorks)
{
    EXPECT_THAT(uut->text(data.convId), StrEq(TEST_STRING));
    EXPECT_THAT(uut->text(42), StrEq(""));
}

K_TEST_F(ApiDrafts, setTextUpdatesText)
{
    const std::string NEW_TEXT = "new text";

    ASSERT_THAT(uut->text(data.convId), Not(StrEq(NEW_TEXT)));
    uut->setText(data.convId, NEW_TEXT);
    EXPECT_THAT(uut->text(data.convId), StrEq(NEW_TEXT));
}

K_TEST_F(ApiDrafts, setTextCreatesNewDraft)
{
    const std::string NEW_TEXT = "another new text";

    ASSERT_THAT(uut->text(42), StrEq(""));
    uut->setText(42, NEW_TEXT);
    EXPECT_THAT(uut->text(42), StrEq(NEW_TEXT));
}

K_TEST_F(ApiDrafts, stateWorks)
{
    EXPECT_THAT(uut->state(data.convId), Eq(Api::DraftState::Editing));
    EXPECT_THAT(uut->state(42), Eq(Api::DraftState::Editing));
}

K_TEST_F(ApiDrafts, prepareToSendThrowsOnEmptySenderName)
{
    settings_->setName("");
    EXPECT_THROW(uut->prepareToSend(data.convId), std::exception);
}

K_TEST_F(ApiDrafts, prepareToSendWorks)
{
    uut->prepareToSend(data.convId);
    uut->prepareToSend(42);
}

K_TEST_F(ApiDrafts, clearWorks)
{
    uut->clear(data.convId);
    EXPECT_THAT(uut->text(data.convId), StrEq(""));
}

K_TEST_F(ApiDrafts, idRangeWorks)
{
    const std::string NEW_TEXT = "another new text";

    for (auto id : TEST_IDS_VALID)
    {
        EXPECT_NO_THROW(uut->state(id));
        EXPECT_NO_THROW(uut->text(id));
        EXPECT_NO_THROW(uut->setText(id, NEW_TEXT));
        EXPECT_NO_THROW(uut->prepareToSend(id));
        EXPECT_NO_THROW(uut->clear(id));
    }

    for (auto id : TEST_IDS_INVALID)
    {
        EXPECT_ANY_THROW(uut->state(id));
        EXPECT_ANY_THROW(uut->text(id));
        EXPECT_ANY_THROW(uut->setText(id, NEW_TEXT));
        EXPECT_ANY_THROW(uut->prepareToSend(id));
        EXPECT_ANY_THROW(uut->clear(id));
    }
}
