/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/dao/usersettingsdao.h>

#include <kulloclient/util/kulloaddress.h>
#include <kulloclient/util/masterkey.h>

#include "tests/dbtest.h"
#include "tests/testdata.h"

using namespace testing;
using namespace Kullo;

namespace {

struct TestData
{
    std::string name = "John Doe";
    std::string organization = "Doe Inc.";
    std::string avatarMimeType = "text/plain";
    std::vector<unsigned char> avatarData = Util::to_vector("Hello, world.");
    std::string footer = "I'm the footer.";
};

}

class UserSettingsDao : public DbTest
{
public:
    UserSettingsDao()
        : uut_(session_)
    {}

protected:
    Util::UserSettings makeUserSettings()
    {
        return Util::UserSettings(
                    Util::Credentials(
                        std::make_shared<Util::KulloAddress>(
                            "johndoe#kullo.net"),
                        std::make_shared<Util::MasterKey>(
                            MasterKeyData::VALID_DATA_BLOCKS)));
    }

    TestData data_;
    Dao::UserSettingsDao uut_;
};

K_TEST_F(UserSettingsDao, loadDoesntThrow)
{
    auto userSettings = makeUserSettings();
    EXPECT_NO_THROW(uut_.load(userSettings));
}

K_TEST_F(UserSettingsDao, setNameWorks)
{
    uut_.setName(data_.name);

    auto userSettings = makeUserSettings();
    uut_.load(userSettings);
    EXPECT_THAT(userSettings.name, Eq(data_.name));
}

K_TEST_F(UserSettingsDao, setOrganizationWorks)
{
    uut_.setOrganization(data_.organization);

    auto userSettings = makeUserSettings();
    uut_.load(userSettings);
    EXPECT_THAT(userSettings.organization, Eq(data_.organization));
}

K_TEST_F(UserSettingsDao, setAvatarMimeTypeWorks)
{
    uut_.setAvatarMimeType(data_.avatarMimeType);

    auto userSettings = makeUserSettings();
    uut_.load(userSettings);
    EXPECT_THAT(userSettings.avatarMimeType, Eq(data_.avatarMimeType));
}

K_TEST_F(UserSettingsDao, setAvatarWorks)
{
    uut_.setAvatarData(data_.avatarData);

    auto userSettings = makeUserSettings();
    uut_.load(userSettings);
    EXPECT_THAT(userSettings.avatarData, Eq(data_.avatarData));
}

K_TEST_F(UserSettingsDao, setFooterWorks)
{
    uut_.setFooter(data_.footer);

    auto userSettings = makeUserSettings();
    uut_.load(userSettings);
    EXPECT_THAT(userSettings.footer, Eq(data_.footer));
}

K_TEST_F(UserSettingsDao, setNextMasterKeyBackupReminderWorks)
{
    auto now = Util::DateTime::nowUtc();

    auto userSettings = makeUserSettings();
    uut_.load(userSettings);
    ASSERT_THAT(userSettings.nextMasterKeyBackupReminder, Not(Eq(boost::none)));
    EXPECT_THAT(*userSettings.nextMasterKeyBackupReminder, Lt(now));

    uut_.setNextMasterKeyBackupReminder(boost::none);

    uut_.load(userSettings);
    EXPECT_THAT(userSettings.nextMasterKeyBackupReminder, Eq(boost::none));

    uut_.setNextMasterKeyBackupReminder(now);

    uut_.load(userSettings);
    ASSERT_THAT(userSettings.nextMasterKeyBackupReminder, Not(Eq(boost::none)));
    EXPECT_THAT(*userSettings.nextMasterKeyBackupReminder, Eq(now));
}
