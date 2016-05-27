/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <kulloclient/api/Address.h>
#include <kulloclient/api/MasterKey.h>
#include <kulloclient/util/stltaskrunner.h>

#include "tests/kullotest.h"

class ApiTest : public KulloTest
{
public:
    ApiTest();
    ~ApiTest() override;

protected:
    std::shared_ptr<Kullo::Api::Address> address_;
    std::shared_ptr<Kullo::Api::MasterKey> masterKey_;

private:
    std::shared_ptr<Kullo::Util::StlTaskRunner> taskRunner_;
};
