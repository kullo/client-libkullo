/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
