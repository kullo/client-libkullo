/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <exception>

#include <kulloclient/util/assert.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

class Assertion : public KulloTest
{
};

K_TEST_F(Assertion, doesntThrowOnSuccess)
{
    ASSERT_NO_THROW(kulloAssert(true));
}

K_TEST_F(Assertion, throwsAssertionFailedOnFailure)
{
    ASSERT_THROW(kulloAssert(false), Util::AssertionFailed);
}

K_TEST_F(Assertion, kulloAssertionFailedUsesMessage)
{
    std::string message = "I'm the message.";
    bool thrown = false;
    try
    {
        kulloAssertionFailed(message);
    }
    catch (Util::AssertionFailed &ex)
    {
        thrown = true;
        ASSERT_THAT(ex.what(), HasSubstr(message));
    }
    ASSERT_THAT(thrown, Eq(true));
}
