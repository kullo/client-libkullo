/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <kulloclient/util/stltaskrunner.h>

#include "tests/kullotest.h"

class ApiTest : public KulloTest
{
public:
    ApiTest();
    ~ApiTest() override;

private:
    std::shared_ptr<Kullo::Util::StlTaskRunner> taskRunner_;
};
