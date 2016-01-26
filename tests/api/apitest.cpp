/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "tests/api/apitest.h"

#include <kulloclient/api/Registry.h>

#include "tests/testutil.h"

using namespace Kullo;

ApiTest::ApiTest()
    : taskRunner_(std::make_shared<Util::StlTaskRunner>())
{
    Api::Registry::setTaskRunner(taskRunner_);
}

ApiTest::~ApiTest()
{
    taskRunner_->wait();
}
