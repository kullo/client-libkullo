/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <kulloclient/api/UserSettings.h>
#include <kulloclient/api/DateTime.h>
#include <kulloclient/util/assert.h>

#include "tests/kullotest.h"
#include "tests/api/mock_address.h"
#include "tests/api/mock_masterkey.h"

using namespace testing;
using namespace Kullo;

namespace {
const std::vector<std::string> VALID_DATA_BLOCKS {
    "000000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000"
};
}

K_TEST(ApiUserSettingsStatic, createThrowsOnNullptr)
{
    std::shared_ptr<Api::Address> addr(new MockAddress());
    std::shared_ptr<Api::MasterKey> key(new MockMasterKey());

    EXPECT_THROW(Api::UserSettings::create(nullptr, nullptr),
                 Util::AssertionFailed);
    EXPECT_THROW(Api::UserSettings::create(addr, nullptr),
                 Util::AssertionFailed);
    EXPECT_THROW(Api::UserSettings::create(nullptr, key),
                 Util::AssertionFailed);
}

K_TEST(ApiUserSettingsStatic, createReturnsNonNull)
{
    auto addr = std::make_shared<NiceMock<MockAddress>>();
    auto key = std::make_shared<NiceMock<MockMasterKey>>();
    ON_CALL(*addr, toString())
            .WillByDefault(Return("example#kullo.net"));
    ON_CALL(*key, dataBlocks())
            .WillByDefault(Return(VALID_DATA_BLOCKS));

    EXPECT_THAT(Api::UserSettings::create(addr, key), Not(IsNull()));
}

class ApiUserSettings : public KulloTest
{
protected:
    ApiUserSettings()
        : address(Api::Address::create("example#kullo.net"))
        , masterKey(Api::MasterKey::createFromDataBlocks(VALID_DATA_BLOCKS))
        , uut(Api::UserSettings::create(address, masterKey))
    {}

    std::shared_ptr<Api::Address> address;
    std::shared_ptr<Api::MasterKey> masterKey;
    std::shared_ptr<Api::UserSettings> uut;
};

K_TEST_F(ApiUserSettings, addressWorks)
{
    EXPECT_THAT(uut->address()->isEqualTo(address), Eq(true));
}

K_TEST_F(ApiUserSettings, masterKeyWorks)
{
    EXPECT_THAT(uut->masterKey()->isEqualTo(masterKey), Eq(true));
}

K_TEST_F(ApiUserSettings, nameWorks)
{
    std::string name = "Arno Nyhm";
    EXPECT_THAT(uut->name(), StrEq(""));
    EXPECT_NO_THROW(uut->setName(name));
    EXPECT_THAT(uut->name(), StrEq(name));
}

K_TEST_F(ApiUserSettings, organizationWorks)
{
    std::string organization = "Kullo GmbH";
    EXPECT_THAT(uut->organization(), StrEq(""));
    EXPECT_NO_THROW(uut->setOrganization(organization));
    EXPECT_THAT(uut->organization(), StrEq(organization));
}

K_TEST_F(ApiUserSettings, footerWorks)
{
    std::string footer = "Bye, world.";
    EXPECT_THAT(uut->footer(), StrEq(""));
    EXPECT_NO_THROW(uut->setFooter(footer));
    EXPECT_THAT(uut->footer(), StrEq(footer));
}

K_TEST_F(ApiUserSettings, avatarMimeTypeWorks)
{
    std::string avatarMimeType = "image/jpeg";
    EXPECT_THAT(uut->avatarMimeType(), StrEq(""));
    EXPECT_NO_THROW(uut->setAvatarMimeType(avatarMimeType));
    EXPECT_THAT(uut->avatarMimeType(), StrEq(avatarMimeType));
}

K_TEST_F(ApiUserSettings, avatarWorks)
{
    std::vector<uint8_t> avatar = {'a', 's', 'd', 'f'};
    EXPECT_THAT(uut->avatar(), Eq(std::vector<uint8_t>()));
    EXPECT_NO_THROW(uut->setAvatar(avatar));
    EXPECT_THAT(uut->avatar(), Eq(avatar));
}

K_TEST_F(ApiUserSettings, keyBackupConfirmedWorks)
{
    EXPECT_THAT(uut->keyBackupConfirmed(), Eq(false));
    EXPECT_NO_THROW(uut->setKeyBackupConfirmed());
    EXPECT_THAT(uut->keyBackupConfirmed(), Eq(true));
    EXPECT_THAT(uut->keyBackupDontRemindBefore().is_initialized(), Eq(false));
}

K_TEST_F(ApiUserSettings, keyBackupDontRemindBeforeWorks)
{
    auto keyBackupDontRemindBefore = Api::DateTime::nowUtc();
    EXPECT_THAT(uut->keyBackupDontRemindBefore(),
                Lt(keyBackupDontRemindBefore));
    EXPECT_NO_THROW(uut->setKeyBackupDontRemindBefore(keyBackupDontRemindBefore));
    EXPECT_THAT(uut->keyBackupDontRemindBefore(),
                Eq(keyBackupDontRemindBefore));
}
