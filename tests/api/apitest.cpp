/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "tests/api/apitest.h"

#include <kulloclient/api/Registry.h>

#include "tests/testdata.h"
#include "tests/testutil.h"

using namespace Kullo;

ApiTest::ApiTest()
    : address_(Api::Address::create("exists#example.com"))
    , masterKey_(
          Api::MasterKey::createFromDataBlocks(
              MasterKeyData::VALID_DATA_BLOCKS))
    , taskRunner_(std::make_shared<Util::StlTaskRunner>())
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
