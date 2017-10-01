/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <kulloclient/api_impl/Address.h>
#include <kulloclient/api_impl/clientcreatesessionworker.h>
#include <kulloclient/api_impl/MasterKey.h>
#include <kulloclient/api_impl/usersettingsimpl.h>
#include <kulloclient/util/kulloaddress.h>
#include <kulloclient/util/masterkey.h>
#include <kulloclient/util/usersettings.h>

#include "tests/testdata.h"
#include "tests/testutil.h"

using namespace testing;
using namespace Kullo;

ApiModelTest::ApiModelTest()
    : sessionConfig_(
          TestUtil::tempDbFileName(),
          Util::Credentials(
              std::make_shared<Util::KulloAddress>(address_.toString()),
              std::make_shared<Util::MasterKey>(masterKey_.blocks)))
{}

void ApiModelTest::makeSession()
{
    sessionListener_ = std::make_shared<StubSessionListener>();
    session_ = ApiImpl::ClientCreateSessionWorker(
                address_, masterKey_, sessionConfig_.dbFileName, sessionListener_, nullptr, boost::none
                ).makeSession().as_nullable();
    sessionListener_->setSession(session_);
    session_->userSettings()->setName("X. Ample User");
}
