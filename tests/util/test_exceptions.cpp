/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/util/exceptions.h>

#include "tests/kullotest.h"

using namespace testing;

class ExceptionFormatter : public KulloTest
{
};

K_TEST_F(ExceptionFormatter, shouldIncludeExceptionName)
{
    std::runtime_error ex("Some error");

    // Don't include "std::" because demangling doesn't work on all platforms
    EXPECT_THAT(Kullo::Util::formatException(ex), HasSubstr("runtime_error"));
}

K_TEST_F(ExceptionFormatter, shouldIncludeExceptionMessage)
{
    std::string message("Some error");
    std::runtime_error ex(message);

    EXPECT_THAT(Kullo::Util::formatException(ex), HasSubstr(message));
}

K_TEST_F(ExceptionFormatter, shouldIncludeNestedExceptionInfo)
{
    std::string outerMessage("outer");
    std::string innerMessage("inner");

    bool caught = false;
    try
    {
        try
        {
            throw std::runtime_error(innerMessage);
        }
        catch (...)
        {
            std::throw_with_nested(std::logic_error(outerMessage));
        }
    }
    catch (std::logic_error &ex)
    {
        caught = true;
        EXPECT_THAT(Kullo::Util::formatException(ex), HasSubstr("std::runtime_error"));
        EXPECT_THAT(Kullo::Util::formatException(ex), HasSubstr(innerMessage));
    }
    EXPECT_THAT(caught, Eq(true));
}


class BaseException : public KulloTest
{
};

K_TEST_F(BaseException, shouldReturnMessageInWhat)
{
    std::string message("This is a test");
    Kullo::Util::BaseException ex(message);

    EXPECT_THAT(ex.what(), StrEq(message));
}
