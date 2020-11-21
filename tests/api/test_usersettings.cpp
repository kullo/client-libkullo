/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/api/UserSettings.h>
#include <kulloclient/api_impl/Address.h>
#include <kulloclient/api_impl/DateTime.h>
#include <kulloclient/util/assert.h>

#include "tests/api/apimodeltest.h"

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

class ApiUserSettings : public ApiModelTest
{
protected:
    ApiUserSettings()
    {
        makeSession();
        uut = session_->userSettings().as_nullable();
    }

    std::shared_ptr<Api::UserSettings> uut;
};

K_TEST_F(ApiUserSettings, addressWorks)
{
    EXPECT_THAT(uut->address(), Eq(address_));
}

K_TEST_F(ApiUserSettings, masterKeyWorks)
{
    EXPECT_THAT(uut->masterKey(), Eq(masterKey_));
}

K_TEST_F(ApiUserSettings, nameWorks)
{
    std::string name = "Arno Nyhm";
    EXPECT_THAT(uut->name(), Not(StrEq(name)));
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

K_TEST_F(ApiUserSettings, keyBackupDontRemindBeforeWorks)
{
    auto keyBackupDontRemindBefore = Api::DateTime::nowUtc();
    EXPECT_THAT(uut->nextMasterKeyBackupReminder(),
                Lt(keyBackupDontRemindBefore));
    EXPECT_NO_THROW(uut->setNextMasterKeyBackupReminder(keyBackupDontRemindBefore));
    EXPECT_THAT(uut->nextMasterKeyBackupReminder(),
                Eq(keyBackupDontRemindBefore));
}
