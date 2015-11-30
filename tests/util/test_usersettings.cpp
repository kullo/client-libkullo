/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include <kulloclient/util/binary.h>
#include <kulloclient/util/datetime.h>
#include <kulloclient/util/kulloaddress.h>
#include <kulloclient/util/masterkey.h>
#include <kulloclient/util/usersettings.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

namespace {
    struct TestData
    {
        const Util::KulloAddress address = Util::KulloAddress("test#kullo.net");
        const std::string name = "Mr. X";
        const std::string organization = "XY Corp.";
        const std::string footer = "Goodbye, world.";
    };
}

class UserSettings : public KulloTest
{
};

K_TEST_F(UserSettings, empty)
{
    auto settings = Util::UserSettings();
    EXPECT_THAT(settings.name, Eq(""));
    EXPECT_THAT(settings.organization, Eq(""));
    EXPECT_THAT(settings.avatarMimeType, Eq(""));
    EXPECT_THAT(settings.avatarData, Eq(Util::to_vector("")));
    EXPECT_THAT(settings.footer, Eq(""));
    EXPECT_THAT(settings.address, IsNull());
    EXPECT_THAT(settings.masterKeyBackupConfirmed(), Eq(false));
    EXPECT_THAT(settings.masterKeyBackupDontRemindBefore(), Eq(Util::DateTime::epoch()));
}

K_TEST_F(UserSettings, setAddress)
{
    TestData data;
    auto settings = Util::UserSettings();
    EXPECT_THAT(settings.address, IsNull());
    settings.address = std::make_shared<Util::KulloAddress>(data.address);
    EXPECT_THAT(*settings.address, Eq(data.address));
}

K_TEST_F(UserSettings, setMasterKey)
{
    auto masterKey = Util::MasterKey::makeRandom();
    auto settings = Util::UserSettings();
    EXPECT_THAT(settings.masterKey, IsNull());
    settings.masterKey = std::make_shared<Util::MasterKey>(masterKey);
    EXPECT_THAT(*settings.masterKey, Eq(masterKey));
}

K_TEST_F(UserSettings, setSenderInformation)
{
    TestData data;
    auto settings = Util::UserSettings();
    settings.name = data.name;
    settings.organization = data.organization;
    settings.footer = data.footer;

    EXPECT_THAT(settings.name, Eq(data.name));
    EXPECT_THAT(settings.organization, Eq(data.organization));
    EXPECT_THAT(settings.footer, Eq(data.footer));
}

K_TEST_F(UserSettings, setMastKeyBackupConfirmed)
{
    auto settings = Util::UserSettings();
    EXPECT_THAT(settings.masterKeyBackupConfirmed(), Eq(false));
    settings.setMasterKeyBackupConfirmed();
    EXPECT_THAT(settings.masterKeyBackupConfirmed(), Eq(true));
}

K_TEST_F(UserSettings, setMasterKeyBackupDontRemindBefore)
{
    auto now = Util::DateTime::nowUtc();
    auto settings = Util::UserSettings();
    EXPECT_THAT(settings.masterKeyBackupDontRemindBefore(), Eq(Util::DateTime::epoch()));
    settings.setMasterKeyBackupDontRemindBefore(now);
    EXPECT_THAT(settings.masterKeyBackupDontRemindBefore(), Eq(now));
}

K_TEST_F(UserSettings, setMasterKeyBackupOrder)
{
    auto now = Util::DateTime::nowUtc();

    {
        auto settings = Util::UserSettings();
        settings.setMasterKeyBackupConfirmed();
        settings.setMasterKeyBackupDontRemindBefore(now);
        EXPECT_THAT(settings.masterKeyBackupConfirmed(), Eq(false));
        EXPECT_THAT(settings.masterKeyBackupDontRemindBefore(), Eq(now));
    }
    {
        auto settings = Util::UserSettings();
        settings.setMasterKeyBackupDontRemindBefore(now);
        settings.setMasterKeyBackupConfirmed();
        EXPECT_THAT(settings.masterKeyBackupConfirmed(), Eq(true));
        EXPECT_THAT(settings.masterKeyBackupDontRemindBefore().isNull(), Eq(true));
    }
}
