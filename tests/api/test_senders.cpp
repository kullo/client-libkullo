/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "tests/api/apimodeltest.h"

#include <kulloclient/api/Senders.h>
#include <kulloclient/api_impl/Address.h>
#include <kulloclient/crypto/hasher.h>
#include <kulloclient/dao/avatardao.h>
#include <kulloclient/dao/messagedao.h>
#include <kulloclient/dao/senderdao.h>
#include <kulloclient/util/binary.h>

using namespace testing;
using namespace Kullo;

class ApiSenders : public ApiModelTest
{
public:
    ApiSenders()
    {
        data.msgId = 1;
        data.name = "Paula Example";
        auto address = Util::KulloAddress("paula#example.com");
        data.address = Api::Address(address);
        data.organization = "Example Corp.";
        data.avatarMimeType = "image/png";
        data.avatar = Util::to_vector("This is a fake avatar");

        messageData.id = data.msgId;
        messageData.sender = address.toString();

        dbSession_ = Db::makeSession(sessionConfig_);
        Db::migrate(dbSession_);

        auto avatarHash = Dao::AvatarDao::store(data.avatar, dbSession_);

        Dao::SenderDao dao(address, dbSession_);
        dao.setMessageId(data.msgId);
        dao.setName(data.name);
        dao.setOrganization(data.organization);
        dao.setAvatarMimeType(data.avatarMimeType);
        dao.setAvatarHash(avatarHash);
        dao.save();

        Dao::MessageDao messageDao(dbSession_);
        messageDao.setId(messageData.id);
        messageDao.setSender(messageData.sender);
        messageDao.save(Dao::CreateOld::No);

        makeSession();
        uut = session_->senders().as_nullable();
    }

protected:
    struct
    {
        int64_t msgId;
        std::string name;
        boost::optional<Api::Address> address;
        std::string organization;
        std::string avatarMimeType;
        std::vector<uint8_t> avatar;
    } data;

    struct
    {
        int64_t id;
        std::string sender;
    } messageData;

    Db::SharedSessionPtr dbSession_;
    std::shared_ptr<Api::Senders> uut;
};

K_TEST_F(ApiSenders, nameWorks)
{
    EXPECT_THAT(uut->name(data.msgId), Eq(data.name));

    EXPECT_THAT(uut->name(42), IsEmpty());
}

K_TEST_F(ApiSenders, addressWorks)
{
    EXPECT_THAT(uut->address(data.msgId), Eq(data.address));

    EXPECT_THAT(uut->address(42), Eq(boost::none));
}

K_TEST_F(ApiSenders, organizationWorks)
{
    EXPECT_THAT(uut->organization(data.msgId), Eq(data.organization));

    EXPECT_THAT(uut->organization(42), IsEmpty());
}

K_TEST_F(ApiSenders, avatarMimeTypeWorks)
{
    EXPECT_THAT(uut->avatarMimeType(data.msgId), Eq(data.avatarMimeType));

    EXPECT_THAT(uut->avatarMimeType(42), IsEmpty());
}

K_TEST_F(ApiSenders, avatarWorks)
{
    EXPECT_THAT(uut->avatar(data.msgId), Eq(data.avatar));

    EXPECT_THAT(uut->avatar(42), IsEmpty());
}

K_TEST_F(ApiSenders, idRangeWorks)
{
    for (auto id : TEST_IDS_VALID)
    {
        EXPECT_NO_THROW(uut->name(id));
        EXPECT_NO_THROW(uut->address(id));
        EXPECT_NO_THROW(uut->organization(id));
        EXPECT_NO_THROW(uut->avatarMimeType(id));
        EXPECT_NO_THROW(uut->avatar(id));
    }

    for (auto id : TEST_IDS_INVALID)
    {
        EXPECT_ANY_THROW(uut->name(id));
        EXPECT_ANY_THROW(uut->address(id));
        EXPECT_ANY_THROW(uut->organization(id));
        EXPECT_ANY_THROW(uut->avatarMimeType(id));
        EXPECT_ANY_THROW(uut->avatar(id));
    }
}
