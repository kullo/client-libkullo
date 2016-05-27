/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <kulloclient/api/Address.h>
#include <kulloclient/api/MasterKey.h>
#include <kulloclient/api_impl/clientcreatesessionworker.h>
#include <kulloclient/api_impl/usersettingsimpl.h>

#include "tests/testutil.h"

using namespace testing;
using namespace Kullo;

ApiModelTest::ApiModelTest()
    : dbPath_(TestUtil::tempDbFileName())
{}

void ApiModelTest::makeSession()
{
    sessionListener_ = std::make_shared<StubSessionListener>();
    session_ = ApiImpl::ClientCreateSessionWorker(
                address_, masterKey_, dbPath_, sessionListener_, nullptr
                ).makeSession();
    sessionListener_->setSession(session_);
    session_->userSettings()->setName("X. Ample User");
}
