/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "tests/api/apitest.h"

#include <kulloclient/api/Registry.h>

#include "tests/testdata.h"
#include "tests/testutil.h"

using namespace Kullo;

ApiTest::ApiTest()
    : address_("exists#example.com")
    , masterKey_(MasterKeyData::VALID_DATA_BLOCKS)
    , taskRunner_(nn_make_shared<Util::StlTaskRunner>())
{
    Api::Registry::setTaskRunner(taskRunner_);
}

ApiTest::~ApiTest()
{
    try
    {
        taskRunner_->wait();
    }
    catch (const std::exception &ex)
    {
        Log.e() << "TaskRunner.wait() failed in ~ApiTest: " << ex.what();
    }
}
