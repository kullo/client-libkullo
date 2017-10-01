/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <kulloclient/kulloclient-forwards.h>
#include <kulloclient/api_impl/Address.h>
#include <kulloclient/api_impl/MasterKey.h>
#include <kulloclient/util/stltaskrunner.h>

#include "tests/kullotest.h"

class ApiTest : public KulloTest
{
public:
    ApiTest();
    ~ApiTest() override;

protected:
    const Kullo::Api::Address address_;
    const Kullo::Api::MasterKey masterKey_;

private:
    Kullo::nn_shared_ptr<Kullo::Util::StlTaskRunner> taskRunner_;
};
