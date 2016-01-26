/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <kulloclient/api/Address.h>
#include <kulloclient/api/MasterKey.h>
#include <kulloclient/api_impl/clientcreatesessionworker.h>
#include <kulloclient/api_impl/usersettingsimpl.h>

#include "tests/testdata.h"
#include "tests/testutil.h"

using namespace testing;
using namespace Kullo;

ApiModelTest::ApiModelTest()
    : dbPath_(TestUtil::tempDbFileName())
{
    auto address = Api::Address::create("exists#example.com");
    auto masterKey = Api::MasterKey::createFromDataBlocks(
                MasterKeyData::VALID_DATA_BLOCKS);
    settings_ = Api::UserSettings::create(address, masterKey);
    settings_->setName("X. Ample User");
}

void ApiModelTest::makeSession()
{
    sessionListener_ = std::make_shared<StubSessionListener>();
    auto userSettingsImpl =
            std::dynamic_pointer_cast<ApiImpl::UserSettingsImpl>(settings_);
    kulloAssert(userSettingsImpl);
    session_ = ApiImpl::ClientCreateSessionWorker(
                userSettingsImpl, dbPath_, sessionListener_, nullptr
                ).makeSession();
    sessionListener_->setSession(session_);
}
