/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/util/binary.h>
#include <kulloclient/util/datetime.h>
#include <kulloclient/util/kulloaddress.h>
#include <kulloclient/util/masterkey.h>
#include <kulloclient/util/usersettings.h>

#include "tests/kullotest.h"
#include "tests/testdata.h"

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
public:
    UserSettings()
        : uut_(
              Util::Credentials(
                  std::make_shared<Util::KulloAddress>(testData_.address),
                  std::make_shared<Util::MasterKey>(MasterKeyData::VALID_DATA_BLOCKS)))
    {}

protected:
    TestData testData_;
    Util::UserSettings uut_;
};

K_TEST_F(UserSettings, empty)
{
    EXPECT_THAT(uut_.name, Eq(""));
    EXPECT_THAT(uut_.organization, Eq(""));
    EXPECT_THAT(uut_.avatarMimeType, Eq(""));
    EXPECT_THAT(uut_.avatarData, Eq(Util::to_vector("")));
    EXPECT_THAT(uut_.footer, Eq(""));
    ASSERT_THAT(uut_.nextMasterKeyBackupReminder.is_initialized(), Eq(true));
    EXPECT_THAT(*uut_.nextMasterKeyBackupReminder, Eq(Util::DateTime::epoch()));
}

K_TEST_F(UserSettings, setSenderInformation)
{
    TestData data;
    uut_.name = data.name;
    uut_.organization = data.organization;
    uut_.footer = data.footer;

    EXPECT_THAT(uut_.name, Eq(data.name));
    EXPECT_THAT(uut_.organization, Eq(data.organization));
    EXPECT_THAT(uut_.footer, Eq(data.footer));
}

K_TEST_F(UserSettings, setNextMasterKeyBackupReminder)
{
    auto now = Util::DateTime::nowUtc();
    ASSERT_THAT(uut_.nextMasterKeyBackupReminder, Not(Eq(boost::none)));
    EXPECT_THAT(*uut_.nextMasterKeyBackupReminder, Lt(now));
    uut_.nextMasterKeyBackupReminder = now;
    EXPECT_THAT(*uut_.nextMasterKeyBackupReminder, Eq(now));
}
