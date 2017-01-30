/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <kulloclient/api/Session.h>
#include <kulloclient/api/SessionListener.h>
#include <kulloclient/db/dbsession.h>
#include <kulloclient/types.h>

#include "tests/api/apitest.h"
#include "tests/api/stub_sessionlistener.h"

class ApiModelTest : public ApiTest
{
public:
    ApiModelTest();

protected:
    void makeSession();

    // Work around C2797: list initialization inside member initializer list or non-static data member initializer is not implemented
    // https://msdn.microsoft.com/en-us/library/dn793970.aspx
    const std::vector<Kullo::id_type> TEST_IDS_VALID = std::vector<Kullo::id_type>{
            0, 1, 42,
            4294967295, // max uint32
            4294967296, // max uint32 + 1
            9007199254740992ll // 2^53
    };
    const std::vector<Kullo::id_type> TEST_IDS_INVALID = std::vector<Kullo::id_type>{
        -1, -100, 9007199254740993ll};

    Kullo::Db::SessionConfig sessionConfig_;
    std::shared_ptr<Kullo::Api::Session> session_;
    std::shared_ptr<StubSessionListener> sessionListener_;
};
