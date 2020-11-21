/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "tests/api/apitest.h"

#include <chrono>

#include <kulloclient/registry.h>
#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/api/Client.h>
#include <kulloclient/api/ClientAddressExistsListener.h>
#include <kulloclient/api/ClientCheckCredentialsListener.h>
#include <kulloclient/api/ClientCreateSessionListener.h>
#include <kulloclient/api/LocalError.h>
#include <kulloclient/api/MasterKeyHelpers.h>
#include <kulloclient/api/Session.h>
#include <kulloclient/api/UserSettings.h>
#include <kulloclient/api_impl/Address.h>
#include <kulloclient/api_impl/MasterKey.h>
#include <kulloclient/util/assert.h>
#include <kulloclient/util/misc.h>

#include "tests/testdata.h"
#include "tests/testutil.h"
#include "tests/api/clientgeneratekeyslistener.h"
#include "tests/api/stub_sessionlistener.h"
#include "tests/http/fake_httpclientfactory.h"

using namespace testing;
using namespace Kullo;

K_TEST(ApiClientStatic, createWorks)
{
    // Client instances need an HttpClient implementation to work with
    auto httpClientFactory = std::make_shared<FakeHttpClientFactory>();
    Kullo::Registry::setHttpClientFactory(httpClientFactory);

    EXPECT_THAT(Api::Client::create(), NotNull());
}


namespace {
class LoginListener : public Api::ClientCreateSessionListener
{
public:
    void migrationStarted(const Api::Address &address) override
    {
        (void)address;
        didMigrate_ = true;
    }

    void finished(const Kullo::nn_shared_ptr<Api::Session> &session) override
    {
        isFinished_ = true;
        session_ = session;
    }

    void error(const Api::Address &address, Api::LocalError error) override
    {
        (void)address;
        (void)error;
    }

    bool didMigrate_ = false;
    bool isFinished_ = false;
    std::shared_ptr<Api::Session> session_;
};

class AddressExistsListener :
        public Api::ClientAddressExistsListener
{
public:
    void finished(const Api::Address &address, bool exists) override
    {
        isFinished_ = true;
        address_ = address;
        exists_ = exists;
    }

    void error(const Api::Address &address, Api::NetworkError error) override
    {
        (void)address;
        (void)error;
    }

    bool isFinished_ = false;
    boost::optional<Api::Address> address_;
    bool exists_ = false;
};

class CheckCredentialsListener :
        public Api::ClientCheckCredentialsListener
{
public:
    void finished(
            const Api::Address &address,
            const Api::MasterKey &masterKey,
            bool valid
            ) override
    {
        isFinished_ = true;
        address_ = address;
        masterKey_ = masterKey;
        valid_ = valid;
    }

    void error(const Api::Address &address, Api::NetworkError error) override
    {
        (void)address;
        (void)error;
    }

    bool isFinished_ = false;
    boost::optional<Api::Address> address_;
    boost::optional<Api::MasterKey> masterKey_;
    bool valid_ = false;
};
}

class ApiClient : public ApiTest
{
protected:
    ApiClient()
        : uut(Api::Client::create())
    {
        auto httpClientFactory = std::make_shared<FakeHttpClientFactory>();
        Registry::setHttpClientFactory(httpClientFactory);

        dbPath = TestUtil::tempDbFileName();
    }

    void addressExistsTest(const std::string &addrStr, bool exists)
    {
        auto address = Api::Address(addrStr);
        auto listener = Kullo::nn_make_shared<AddressExistsListener>();
        auto task = uut->addressExistsAsync(address, listener);

        ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                    Eq(TestUtil::OK));
        EXPECT_THAT(listener->address_, Eq(address));
        EXPECT_THAT(listener->exists_, Eq(exists));
    }

    void checkCredentialsTest(
            const std::string &addrStr,
            const std::vector<std::string> &masterKeyBlocks,
            bool valid)
    {
        auto address = Api::Address(addrStr);
        auto masterKey = Api::MasterKey(masterKeyBlocks);
        auto listener = Kullo::nn_make_shared<CheckCredentialsListener>();
        auto task = uut->checkCredentialsAsync(address, masterKey, listener);

        ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                    Eq(TestUtil::OK));
        EXPECT_THAT(listener->address_, Eq(address));
        EXPECT_THAT(listener->masterKey_, Eq(masterKey));
        EXPECT_THAT(listener->valid_, Eq(valid));
    }

    std::string dbPath;
    Kullo::nn_shared_ptr<Api::Client> uut;
};

K_TEST_F(ApiClient, createSessionAsyncFailsOnNull)
{
    auto sessionListener = Kullo::nn_make_shared<StubSessionListener>();
    auto listener = Kullo::nn_make_shared<LoginListener>();

    EXPECT_THROW(uut->createSessionAsync(address_, masterKey_, "", sessionListener, listener),
                 Util::AssertionFailed);
}

K_TEST_F(ApiClient, createSessionAsyncCanBeCanceled)
{
    auto sessionListener = Kullo::nn_make_shared<StubSessionListener>();
    auto listener = Kullo::nn_make_shared<LoginListener>();
    auto task = uut->createSessionAsync(address_, masterKey_, dbPath, sessionListener, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiClient, createSessionAsyncWorks)
{
    auto sessionListener = Kullo::nn_make_shared<StubSessionListener>();
    auto listener = Kullo::nn_make_shared<LoginListener>();
    auto task = uut->createSessionAsync(address_, masterKey_, dbPath, sessionListener, listener);

    ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                Eq(TestUtil::OK));
    EXPECT_THAT(listener->didMigrate_, Eq(true));
    EXPECT_THAT(listener->session_, Not(IsNull()));

    // re-run create session on existing DB, should not migrate
    listener = Kullo::nn_make_shared<LoginListener>();
    task = uut->createSessionAsync(address_, masterKey_, dbPath, sessionListener, listener);
    ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                Eq(TestUtil::OK));
    EXPECT_THAT(listener->didMigrate_, Eq(false));
}

K_TEST_F(ApiClient, addressExistsAsyncCanBeCanceled)
{
    auto address = Api::Address("doesntexist#example.com");
    auto listener = Kullo::nn_make_shared<AddressExistsListener>();
    auto task = uut->addressExistsAsync(address, listener);
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiClient, addressExistsAsyncWorksForNonExistingUser)
{
    addressExistsTest("doesntexist#example.com", false);
}

K_TEST_F(ApiClient, addressExistsAsyncWorksForExistingUser)
{
    addressExistsTest("exists#example.com", true);
}

K_TEST_F(ApiClient, checkLoginAsyncCanBeCanceled)
{
    auto address = Api::Address("exists#example.com");
    auto masterKey = Api::MasterKey(MasterKeyData::VALID_DATA_BLOCKS);
    auto listener = Kullo::nn_make_shared<CheckCredentialsListener>();
    auto task = uut->checkCredentialsAsync(address, masterKey, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiClient, checkLoginAsyncWorksForBadLogin)
{
    checkCredentialsTest(
                "exists#example.com",
                MasterKeyData::VALID_DATA_BLOCKS2,
                false);
}

K_TEST_F(ApiClient, checkLoginAsyncWorksForGoodLogin)
{
    checkCredentialsTest(
                "exists#example.com",
                MasterKeyData::VALID_DATA_BLOCKS,
                true);
}

// Run with --gtest_also_run_disabled_tests=1 to enable slow tests
K_TEST_F(ApiClient, DISABLED_generateKeysAsyncCanBeCanceled)
{
    auto listener = Kullo::nn_make_shared<ClientGenerateKeysListener>();
    auto task = uut->generateKeysAsync(listener);
    EXPECT_NO_THROW(task->cancel());
}

// Run with --gtest_also_run_disabled_tests=1 to enable slow tests
K_TEST_F(ApiClient, DISABLED_generateKeysAsyncWorks)
{
    auto listener = Kullo::nn_make_shared<ClientGenerateKeysListener>();
    auto task = uut->generateKeysAsync(listener);

    ASSERT_THAT(TestUtil::waitAndCheckSlow(task, listener->isFinished_),
                Eq(TestUtil::OK));
}

K_TEST_F(ApiClient, versionsIsNonempty)
{
    EXPECT_THAT(uut->versions(), Not(IsEmpty()));
}

K_TEST_F(ApiClient, versionsContainsLibkullo)
{
    auto versions = uut->versions();
    auto libIter = versions.find(Api::Client::LIBKULLO);
    ASSERT_THAT(libIter, Ne(versions.end()));
    EXPECT_THAT(libIter->second, Not(IsEmpty()));
}
