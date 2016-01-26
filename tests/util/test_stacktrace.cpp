/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <kulloclient/util/stacktrace.h>
#include <kulloclient/util/misc.h>

#include "tests/kullotest.h"

using namespace testing;

class Stacktrace : public KulloTest
{
};

K_TEST_F(Stacktrace, toString)
{
    // Stacktrace is not available (empty string) on Windows and Android
    // So this is some nice-to-have functionality. Just run it but don't
    // check anything.
    auto stacktrace = Kullo::Util::Stacktrace::toString();
    K_UNUSED(stacktrace);
}

K_TEST_F(Stacktrace, DISABLED_toStdErr)
{
    EXPECT_NO_THROW(Kullo::Util::Stacktrace::toStdErr());
}
