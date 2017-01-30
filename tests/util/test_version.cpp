/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include <kulloclient/util/version.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

class Version : public KulloTest
{
};

#ifdef __APPLE__
K_TEST_F(Version, osVersionApple)
{
    std::string version = Util::Version::osVersionApple();
    // iOS 8.x–11.x or macOS 10.x
    EXPECT_THAT(version, MatchesRegex("(8|9|10|11)\\.[0-9]+"));
}
#endif
