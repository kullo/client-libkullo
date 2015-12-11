/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <kulloclient/api/Senders.h>
#include <kulloclient/api_impl/addressimpl.h>
#include <kulloclient/dao/messagedao.h>
#include <kulloclient/dao/participantdao.h>
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
        data.address = std::make_shared<ApiImpl::AddressImpl>(address);
        data.organization = "Example Corp.";
        data.avatarMimeType = "image/png";
        data.avatar = Util::to_vector("This is a fake avatar");

        messageData.id = data.msgId;
        messageData.sender = address.toString();

        dbSession_ = Db::makeSession(dbPath_);
        Db::migrate(dbSession_);

        Dao::ParticipantDao dao(address, dbSession_);
        dao.setMessageId(data.msgId);
        dao.setName(data.name);
        dao.setOrganization(data.organization);
        dao.setAvatarMimeType(data.avatarMimeType);
        dao.setAvatar(data.avatar);
        dao.save();

        Dao::MessageDao messageDao(dbSession_);
        messageDao.setId(messageData.id);
        messageDao.setSender(messageData.sender);
        messageDao.save(Dao::CreateOld::No);

        makeSession();
        uut = session_->senders();
    }

protected:
    struct
    {
        int64_t msgId;
        std::string name;
        std::shared_ptr<Api::Address> address;
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
    EXPECT_THAT(uut->address(data.msgId)->isEqualTo(data.address), Eq(true));

    EXPECT_THAT(uut->address(42), IsNull());
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
