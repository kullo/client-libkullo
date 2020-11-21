/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/dao/messagedao.h>
#include <kulloclient/dao/senderdao.h>
#include <kulloclient/util/hex.h>

#include "tests/dbtest.h"

using namespace testing;
using namespace Kullo;

namespace {
    struct TestData
    {
        const std::string address = "test#kullo.net";
        const std::string name = "Me. Smith";
        const std::string organization = "Smith&Smith Ltd.";
        const id_type messageId = 42;
        const std::string avatarMimeType = "image/png";
        const int64_t avatarHash = 1337;
    };
}

class ParticipantDao : public DbTest
{
};

K_TEST_F(ParticipantDao, insertsNewParticipant)
{
    TestData data;

    auto dao = Dao::SenderDao(Util::KulloAddress(data.address), session_);
    dao.setMessageId(data.messageId);
    EXPECT_THAT(dao.save(), Eq(true));
    EXPECT_THAT(dao.save(), Eq(false));
}

K_TEST_F(ParticipantDao, doesntModifySetData)
{
    TestData data;

    auto dao = Dao::SenderDao(Util::KulloAddress(data.address), session_);
    dao.setName(data.name);
    dao.setOrganization(data.organization);
    dao.setMessageId(data.messageId);
    dao.setAvatarMimeType(data.avatarMimeType);
    dao.setAvatarHash(data.avatarHash);

    EXPECT_THAT(dao.address(), Eq(Util::KulloAddress(data.address)));
    EXPECT_THAT(dao.name(), Eq(data.name));
    EXPECT_THAT(dao.organization(), Eq(data.organization));
    EXPECT_THAT(dao.messageId(), Eq(data.messageId));
    EXPECT_THAT(dao.avatarMimeType(), Eq(data.avatarMimeType));
    EXPECT_THAT(dao.avatarHash(), Eq(data.avatarHash));
}

K_TEST_F(ParticipantDao, loadedParticipantIsNotDirty)
{
    const auto senderAddress = Util::KulloAddress("sender#kullo.net");
    const auto messageId = 1;

    // Store in db
    {
        auto msg = Dao::MessageDao(session_);
        msg.setSender(senderAddress.toString());
        msg.setId(messageId);
        msg.save(Dao::CreateOld::No);
        auto sender = Dao::SenderDao(senderAddress, session_);
        sender.setMessageId(messageId);
        sender.save();
    }

    // Read from db
    {
        auto sender = Dao::SenderDao::load(messageId, session_);
        ASSERT_THAT(sender, NotNull());
        EXPECT_THAT(sender->save(), Eq(false));
    }
}
