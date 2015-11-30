/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include <kulloclient/dao/participantdao.h>
#include <kulloclient/dao/messagedao.h>
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
        // cat test-avatar.png | hexdump -v -e '/1 "%02X"'; echo
        const std::vector<unsigned char> avatar = Util::Hex::decode(
                "89504E470D0A1A0A0000000D4948445200000050000000500802000000017"
                "365FA000000097048597300000B1300000B1301009A9C180000000774494D"
                "4507DF0105100F35E80FA6440000001974455874436F6D6D656E740043726"
                "56174656420776974682047494D5057810E17000000674944415478DAEDCF"
                "31010000040030DAAAE7954E0FB606CBA98E4F52585858585858585858585"
                "8585858585858585858585858585858585858585858585858585858585858"
                "5858585858585858585858585858585858585858585858585858585858585"
                "8F892058218924163BADE480000000049454E44AE426082"
                );
    };
}

class ParticipantDao : public DbTest
{
};

K_TEST_F(ParticipantDao, insertsNewParticipant)
{
    TestData data;

    auto dao = Dao::ParticipantDao(Util::KulloAddress(data.address), session_);
    EXPECT_THAT(dao.save(), Eq(true));
    EXPECT_THAT(dao.save(), Eq(false));
}

K_TEST_F(ParticipantDao, doesntModifySetData)
{
    TestData data;

    auto dao = Dao::ParticipantDao(Util::KulloAddress(data.address), session_);
    dao.setName(data.name);
    dao.setOrganization(data.organization);
    dao.setMessageId(data.messageId);
    dao.setAvatarMimeType(data.avatarMimeType);
    dao.setAvatar(data.avatar);

    EXPECT_THAT(dao.address(), Eq(Util::KulloAddress(data.address)));
    EXPECT_THAT(dao.name(), Eq(data.name));
    EXPECT_THAT(dao.organization(), Eq(data.organization));
    EXPECT_THAT(dao.messageId(), Eq(data.messageId));
    EXPECT_THAT(dao.avatarMimeType(), Eq(data.avatarMimeType));
    EXPECT_THAT(dao.avatar(), Eq(data.avatar));
}

K_TEST_F(ParticipantDao, loadedParticipantIsNotDirty)
{
    const auto senderAddress   = Util::KulloAddress("sender#kullo.net");

    // Store in db
    {
        auto msg = Dao::MessageDao(session_);
        msg.setSender(senderAddress.toString());
        msg.setId(1);
        msg.save(Dao::CreateOld::No);
        auto sender = Dao::ParticipantDao(senderAddress, session_);
        sender.setMessageId(1);
        sender.save();
    }

    // Read from db
    {
        auto sender = Dao::ParticipantDao::load(senderAddress, session_);
        ASSERT_THAT(sender, NotNull());
        EXPECT_THAT(sender->save(), Eq(false));
    }
}
