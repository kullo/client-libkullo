/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
// must be included before anything that includes LibraryLogger due to "Log" name clash
#include "tests/kullotest.h"

#include <kulloclient/api/LogListener.h>
#include <kulloclient/api/Registry.h>
#include <kulloclient/util/librarylogger.h>

#include "tests/api/mock_loglistener.h"

using namespace testing;
using namespace Kullo;

K_TEST(ApiRegistryStatic, setLogListenerWorks)
{
    EXPECT_NO_THROW(Api::Registry::setLogListener(nullptr));

    auto logListener = std::make_shared<MockLogListener>();
    EXPECT_CALL(*logListener, writeLogMessage(_, _, _, _, _, _, _))
            .Times(AtLeast(1));
    EXPECT_NO_THROW(Api::Registry::setLogListener(logListener));
    Log.d() << "ApiRegistryStatic setLogListenerWorks";

    // reset to default because this is global state (cannot be cleaned up by gtest)
    EXPECT_NO_THROW(Api::Registry::setLogListener(nullptr));
}
