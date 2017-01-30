/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "tests/api/apitest.h"

#include <kulloclient/api/Address.h>
#include <kulloclient/api/AddressNotAvailableReason.h>
#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/api/ChallengeType.h>
#include <kulloclient/api/Client.h>
#include <kulloclient/api/NetworkError.h>
#include <kulloclient/api/Registration.h>
#include <kulloclient/api/RegistrationRegisterAccountListener.h>
#include <kulloclient/api_impl/debug.h>
#include <kulloclient/util/assert.h>
#include <kulloclient/registry.h>

#include "tests/api/clientgeneratekeyslistener.h"
#include "tests/http/fake_httpclientfactory.h"
#include "tests/testutil.h"

using namespace testing;
using namespace Kullo;

namespace {
enum struct CallbackType
{
    challengeNeeded,
    addressNotAvailable,
    finished,
    error
};

class RegisterAccountListener :
        public Api::RegistrationRegisterAccountListener
{
public:
    void challengeNeeded(
            const std::shared_ptr<Api::Address> &address,
            const std::shared_ptr<Api::Challenge> &challenge) override
    {
        callback_ = CallbackType::challengeNeeded;
        address_ = address;
        challenge_ = challenge;
    }

    void addressNotAvailable(const std::shared_ptr<Api::Address> &address,
                             Api::AddressNotAvailableReason reason) override
    {
        callback_ = CallbackType::addressNotAvailable;
        address_ = address;
        reason_ = reason;
    }

    void finished(
            const std::shared_ptr<Api::Address> &address,
            const std::shared_ptr<Api::MasterKey> &masterKey) override
    {
        callback_ = CallbackType::finished;
        address_ = address;
        masterKey_ = masterKey;
    }

    void error(
            const std::shared_ptr<Api::Address> &address,
            Api::NetworkError error) override
    {
        kulloAssert(address);
        Log.w() << "RegisterAccountListener::error with "
                << "address = " << address->toString() << ", "
                << "error = " << error;

        callback_ = CallbackType::error;
        address_ = address;
    }

    CallbackType callback_;
    std::shared_ptr<Api::Address> address_;
    std::shared_ptr<Api::Challenge> challenge_;
    std::shared_ptr<Api::MasterKey> masterKey_;
    Api::AddressNotAvailableReason reason_;
};
}

class ApiRegistration : public ApiTest
{
public:
    ApiRegistration()
    {
        reset();

        auto httpClientFactory = std::make_shared<FakeHttpClientFactory>();
        Kullo::Registry::setHttpClientFactory(httpClientFactory);

        auto listener = std::make_shared<ClientGenerateKeysListener>();
        auto task = Api::Client::create()->generateKeysAsync(listener);
        kulloAssert(task->waitForMs(TestUtil::slowTimeoutMs()));
        uut = listener->registration_;
    }

protected:
    void reset()
    {
        address = Api::Address::create("exists#example.com");
        registerAccountListener = std::make_shared<RegisterAccountListener>();
    }

    std::shared_ptr<Api::Address> address;
    std::shared_ptr<RegisterAccountListener> registerAccountListener;
    std::shared_ptr<Api::Registration> uut;
};

// Run with --gtest_also_run_disabled_tests=1 to enable slow tests
K_TEST_F(ApiRegistration, DISABLED_registerAccountAsyncWorks)
{
    // put everything in a single test because generating the keys is so slow

    // FailsOnNull
    {
        // null in third arg (challenge) is allowed
        EXPECT_THROW(uut->registerAccountAsync(nullptr, boost::none, nullptr, "", nullptr),
                     Util::AssertionFailed);
        EXPECT_THROW(uut->registerAccountAsync(address, boost::none, nullptr, "", nullptr),
                     Util::AssertionFailed);
        EXPECT_THROW(uut->registerAccountAsync(nullptr, boost::none, nullptr, "", registerAccountListener),
                     Util::AssertionFailed);
    }
    reset();

    // CanBeCanceled
    {
        auto task = uut->registerAccountAsync(address, boost::none, nullptr, "", registerAccountListener);
        ASSERT_THAT(task, Not(IsNull()));
        EXPECT_NO_THROW(task->cancel());
    }
    reset();

    // AddressBlocked
    {
        address = Api::Address::create("blocked#example.com");
        auto task = uut->registerAccountAsync(address, boost::none, nullptr, "", registerAccountListener);
        ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
        EXPECT_THAT(registerAccountListener->callback_, Eq(CallbackType::addressNotAvailable));
        EXPECT_THAT(registerAccountListener->address_, Eq(address));
        EXPECT_THAT(registerAccountListener->reason_, Eq(Api::AddressNotAvailableReason::Blocked));
    }
    reset();

    // AddressExists
    {
        auto task = uut->registerAccountAsync(address, boost::none, nullptr, "", registerAccountListener);
        ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
        EXPECT_THAT(registerAccountListener->callback_, Eq(CallbackType::addressNotAvailable));
        EXPECT_THAT(registerAccountListener->address_, Eq(address));
        EXPECT_THAT(registerAccountListener->reason_, Eq(Api::AddressNotAvailableReason::Exists));
    }
    reset();

    // WorksWithoutChallenge
    {
        auto address = Api::Address::create("nochallenge#example.com");
        auto task = uut->registerAccountAsync(address, boost::none, nullptr, "", registerAccountListener);
        ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
        EXPECT_THAT(registerAccountListener->callback_, Eq(CallbackType::finished));
        EXPECT_THAT(registerAccountListener->address_, Eq(address));
        EXPECT_THAT(registerAccountListener->masterKey_, Not(IsNull()));
    }
    reset();

    // WorksWithChallenge
    {
        auto address = Api::Address::create("withchallenge#example.com");

        // get challenge
        auto task = uut->registerAccountAsync(address, boost::none, nullptr, "", registerAccountListener);
        ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
        EXPECT_THAT(registerAccountListener->callback_, Eq(CallbackType::challengeNeeded));
        EXPECT_THAT(registerAccountListener->address_, Eq(address));
        EXPECT_THAT(registerAccountListener->challenge_, Not(IsNull()));
        auto challenge = registerAccountListener->challenge_;

        reset();

        // register account
        task = uut->registerAccountAsync(address, boost::none, challenge, "42", registerAccountListener);
        ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
        EXPECT_THAT(registerAccountListener->callback_, Eq(CallbackType::finished));
        EXPECT_THAT(registerAccountListener->address_, Eq(address));
        EXPECT_THAT(registerAccountListener->masterKey_, Not(IsNull()));
    }
}
