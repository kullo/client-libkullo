/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <exception>

#include <kulloclient/util/assert.h>

#include "tests/kullotest.h"

using namespace Kullo::Util;

class Assertion : public KulloTest
{
};

K_TEST_F(Assertion, doesntThrowOnSuccess)
{
    ASSERT_NO_THROW(kulloAssert(true));
}

K_TEST_F(Assertion, throwsAssertionFailedOnFailure)
{
    ASSERT_THROW(kulloAssert(false), AssertionFailed);
}
