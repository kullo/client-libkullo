/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "tests/api/apitest.h"

#include <chrono>

#include <kulloclient/registry.h>
#include <kulloclient/api/Address.h>
#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/api/Client.h>
#include <kulloclient/api/ClientAddressExistsListener.h>
#include <kulloclient/api/ClientCheckLoginListener.h>
#include <kulloclient/api/ClientCreateSessionListener.h>
#include <kulloclient/api/LocalError.h>
#include <kulloclient/api/MasterKey.h>
#include <kulloclient/api/Session.h>
#include <kulloclient/api/UserSettings.h>
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
    void finished(const std::shared_ptr<Api::Session> &session) override
    {
        isFinished_ = true;
        session_ = session;
    }

    void error(
            const std::shared_ptr<Api::Address> &address,
            Api::LocalError error) override
    {
        (void)address;
        (void)error;
    }

    bool isFinished_ = false;
    std::shared_ptr<Api::Session> session_;
};

class AddressExistsListener :
        public Api::ClientAddressExistsListener
{
public:
    void finished(
            const std::shared_ptr<Api::Address> &address,
            bool exists
            ) override
    {
        isFinished_ = true;
        address_ = address;
        exists_ = exists;
    }

    void error(
            const std::shared_ptr<Api::Address> &address,
            Api::NetworkError error) override
    {
        (void)address;
        (void)error;
    }

    bool isFinished_ = false;
    std::shared_ptr<Api::Address> address_;
    bool exists_ = false;
};

class CheckLoginListener :
        public Api::ClientCheckLoginListener
{
public:
    void finished(
            const std::shared_ptr<Api::Address> &address,
            const std::shared_ptr<Api::MasterKey> &masterKey,
            bool valid
            ) override
    {
        isFinished_ = true;
        address_ = address;
        masterKey_ = masterKey;
        valid_ = valid;
    }

    void error(
            const std::shared_ptr<Api::Address> &address,
            Api::NetworkError error) override
    {
        (void)address;
        (void)error;
    }

    bool isFinished_ = false;
    std::shared_ptr<Api::Address> address_;
    std::shared_ptr<Api::MasterKey> masterKey_;
    bool valid_ = false;
};
}

class ApiClient : public ApiTest
{
protected:
    ApiClient()
    {
        auto httpClientFactory = std::make_shared<FakeHttpClientFactory>();
        Registry::setHttpClientFactory(httpClientFactory);

        dbPath = TestUtil::tempDbFileName();

        auto address = Api::Address::create("exists#example.com");
        auto masterKey = Api::MasterKey::createFromDataBlocks(
                    MasterKeyData::VALID_DATA_BLOCKS);
        settings = Api::UserSettings::create(address, masterKey);

        uut = Api::Client::create();
    }

    void addressExistsTest(const std::string &addrStr, bool exists)
    {
        auto addr = Api::Address::create(addrStr);
        auto listener = std::make_shared<AddressExistsListener>();
        auto task = uut->addressExistsAsync(addr, listener);

        ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                    Eq(TestUtil::OK));
        EXPECT_THAT(listener->address_->isEqualTo(addr), Eq(true));
        EXPECT_THAT(listener->exists_, Eq(exists));
    }

    void checkLoginTest(
            const std::string &addrStr,
            const std::vector<std::string> &masterKeyBlocks,
            bool valid)
    {
        auto addr = Api::Address::create(addrStr);
        auto key = Api::MasterKey::createFromDataBlocks(masterKeyBlocks);
        auto listener = std::make_shared<CheckLoginListener>();
        auto task = uut->checkLoginAsync(addr, key, listener);

        ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                    Eq(TestUtil::OK));
        EXPECT_THAT(listener->address_->isEqualTo(addr), Eq(true));
        EXPECT_THAT(listener->masterKey_->isEqualTo(key), Eq(true));
        EXPECT_THAT(listener->valid_, Eq(valid));
    }

    std::string dbPath;
    std::shared_ptr<Api::UserSettings> settings;
    std::shared_ptr<Api::Client> uut;
};

K_TEST_F(ApiClient, createSessionAsyncFailsOnNull)
{
    auto sessionListener = std::make_shared<StubSessionListener>();
    auto listener = std::make_shared<LoginListener>();

    EXPECT_THROW(uut->createSessionAsync(nullptr, "", nullptr, nullptr),
                 Util::AssertionFailed);
    EXPECT_THROW(uut->createSessionAsync(nullptr, dbPath, sessionListener, listener),
                 Util::AssertionFailed);
    EXPECT_THROW(uut->createSessionAsync(settings, "", sessionListener, listener),
                 Util::AssertionFailed);
    EXPECT_THROW(uut->createSessionAsync(settings, dbPath, nullptr, listener),
                 Util::AssertionFailed);
    EXPECT_THROW(uut->createSessionAsync(settings, dbPath, sessionListener, nullptr),
                 Util::AssertionFailed);
}

K_TEST_F(ApiClient, createSessionAsyncCanBeCanceled)
{
    auto sessionListener = std::make_shared<StubSessionListener>();
    auto listener = std::make_shared<LoginListener>();
    auto task = uut->createSessionAsync(settings, dbPath, sessionListener, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiClient, createSessionAsyncWorks)
{
    auto sessionListener = std::make_shared<StubSessionListener>();
    auto listener = std::make_shared<LoginListener>();
    auto task = uut->createSessionAsync(settings, dbPath, sessionListener, listener);

    ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                Eq(TestUtil::OK));
    ASSERT_THAT(listener->session_, Not(IsNull()));
}

K_TEST_F(ApiClient, addressExistsAsyncFailsOnNull)
{
    auto addr = Api::Address::create("doesntexist#example.com");
    auto listener = std::make_shared<AddressExistsListener>();
    EXPECT_THROW(uut->addressExistsAsync(nullptr, nullptr), Util::AssertionFailed);
    EXPECT_THROW(uut->addressExistsAsync(addr, nullptr), Util::AssertionFailed);
    EXPECT_THROW(uut->addressExistsAsync(nullptr, listener), Util::AssertionFailed);
}

K_TEST_F(ApiClient, addressExistsAsyncCanBeCanceled)
{
    auto addr = Api::Address::create("doesntexist#example.com");
    auto listener = std::make_shared<AddressExistsListener>();
    auto task = uut->addressExistsAsync(addr, listener);
    ASSERT_THAT(task, Not(IsNull()));
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

K_TEST_F(ApiClient, checkLoginAsyncFailsOnNull)
{
    auto addr = Api::Address::create("doesntexist#example.com");
    auto key = Api::MasterKey::createFromDataBlocks(
                MasterKeyData::VALID_DATA_BLOCKS);
    auto listener = std::make_shared<CheckLoginListener>();

    EXPECT_THROW(uut->checkLoginAsync(nullptr, nullptr, nullptr), Util::AssertionFailed);
    EXPECT_THROW(uut->checkLoginAsync(addr, nullptr, nullptr), Util::AssertionFailed);
    EXPECT_THROW(uut->checkLoginAsync(nullptr, key, nullptr), Util::AssertionFailed);
    EXPECT_THROW(uut->checkLoginAsync(addr, nullptr, listener), Util::AssertionFailed);
    EXPECT_THROW(uut->checkLoginAsync(nullptr, key, listener), Util::AssertionFailed);
}

K_TEST_F(ApiClient, checkLoginAsyncCanBeCanceled)
{
    auto addr = Api::Address::create("exists#example.com");
    auto key = Api::MasterKey::createFromDataBlocks(
                MasterKeyData::VALID_DATA_BLOCKS);
    auto listener = std::make_shared<CheckLoginListener>();
    auto task = uut->checkLoginAsync(addr, key, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiClient, checkLoginAsyncWorksForBadLogin)
{
    checkLoginTest(
                "exists#example.com",
                MasterKeyData::VALID_DATA_BLOCKS2,
                false);
}

K_TEST_F(ApiClient, checkLoginAsyncWorksForGoodLogin)
{
    checkLoginTest(
                "exists#example.com",
                MasterKeyData::VALID_DATA_BLOCKS,
                true);
}

K_TEST_F(ApiClient, generateKeysAsyncFailsOnNull)
{
    EXPECT_THROW(uut->generateKeysAsync(nullptr), Util::AssertionFailed);
}

// Run with --gtest_also_run_disabled_tests=1 to enable slow tests
K_TEST_F(ApiClient, DISABLED_generateKeysAsyncCanBeCanceled)
{
    auto listener = std::make_shared<ClientGenerateKeysListener>();
    auto task = uut->generateKeysAsync(listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

// Run with --gtest_also_run_disabled_tests=1 to enable slow tests
K_TEST_F(ApiClient, DISABLED_generateKeysAsyncWorks)
{
    auto listener = std::make_shared<ClientGenerateKeysListener>();
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
