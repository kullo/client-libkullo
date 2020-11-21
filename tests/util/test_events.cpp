/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <functional>
#include <string>

#include <kulloclient/util/events.h>
#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo::Util;

class Events : public KulloTest
{
public:
    void foo()
    {
        result = "foo";
    }

    void fooBar()
    {
        result = "fooBar";
    }

protected:
    std::string result;
};

K_TEST_F(Events, areImmediatelyEvaluatedOnDirectAssignment)
{
    std::function<void()> fooEvent = std::bind(&Events::foo, this);
    std::function<void()> barEvent = fooEvent;
    fooEvent = std::bind(&Events::fooBar, this);
    barEvent();

    EXPECT_THAT(result, StrEq("foo"));
}

K_TEST_F(Events, areLazilyEvaluatedOnForward)
{
    std::function<void()> fooEvent = std::bind(&Events::foo, this);
    std::function<void()> barEvent = forwardEvent(fooEvent);
    fooEvent = std::bind(&Events::fooBar, this);
    barEvent();

    EXPECT_THAT(result, StrEq("fooBar"));
}

K_TEST_F(Events, onlyExecuteIfTheyAreDefined)
{
    std::function<void()> event;

    EXPECT_THROW(event(), std::bad_function_call);
    EXPECT_NO_THROW(EMIT0(event));

    auto forwardedEvent = forwardEvent(event);
    EXPECT_NO_THROW(EMIT0(forwardedEvent));
}
