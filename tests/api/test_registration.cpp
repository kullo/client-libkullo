/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "tests/api/apitest.h"

#include <kulloclient/api/AddressNotAvailableReason.h>
#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/api/ChallengeType.h>
#include <kulloclient/api/Client.h>
#include <kulloclient/api/NetworkError.h>
#include <kulloclient/api/Registration.h>
#include <kulloclient/api/RegistrationRegisterAccountListener.h>
#include <kulloclient/api_impl/Address.h>
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
            const Api::Address &address,
            const Kullo::nn_shared_ptr<Api::Challenge> &challenge) override
    {
        callback_ = CallbackType::challengeNeeded;
        address_ = address;
        challenge_ = challenge.as_nullable();
    }

    void addressNotAvailable(
            const Api::Address &address,
            Api::AddressNotAvailableReason reason) override
    {
        callback_ = CallbackType::addressNotAvailable;
        address_ = address;
        reason_ = reason;
    }

    void finished(
            const Api::Address &address,
            const Api::MasterKey &masterKey) override
    {
        callback_ = CallbackType::finished;
        address_ = address;
        masterKey_ = masterKey;
    }

    void error(
            const Api::Address &address,
            Api::NetworkError error) override
    {
        Log.w() << "RegisterAccountListener::error with "
                << "address = " << address << ", "
                << "error = " << error;

        callback_ = CallbackType::error;
        address_ = address;
    }

    CallbackType callback_;
    boost::optional<Api::Address> address_;
    std::shared_ptr<Api::Challenge> challenge_;
    boost::optional<Api::MasterKey> masterKey_;
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

        auto listener = Kullo::nn_make_shared<ClientGenerateKeysListener>();
        auto task = Api::Client::create()->generateKeysAsync(listener);
        kulloAssert(task->waitForMs(TestUtil::slowTimeoutMs()));
        uut = listener->registration_;
    }

protected:
    void reset()
    {
        registerAccountListener = std::make_shared<RegisterAccountListener>();
    }

    std::shared_ptr<RegisterAccountListener> registerAccountListener;
    std::shared_ptr<Api::Registration> uut;
};

// Run with --gtest_also_run_disabled_tests=1 to enable slow tests
K_TEST_F(ApiRegistration, DISABLED_registerAccountAsyncWorks)
{
    // put everything in a single test because generating the keys is so slow

    // CanBeCanceled
    {
        auto address = Api::Address("exists#example.com");
        auto task = uut->registerAccountAsync(
                    address, boost::none, nullptr, "", kulloForcedNn(registerAccountListener));
        ASSERT_THAT(task, Not(IsNull()));
        EXPECT_NO_THROW(task->cancel());
    }
    reset();

    // AddressBlocked
    {
        auto address = Api::Address("blocked#example.com");
        auto task = uut->registerAccountAsync(
                    address, boost::none, nullptr, "", kulloForcedNn(registerAccountListener));
        ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
        EXPECT_THAT(registerAccountListener->callback_, Eq(CallbackType::addressNotAvailable));
        EXPECT_THAT(registerAccountListener->address_, Eq(address));
        EXPECT_THAT(registerAccountListener->reason_, Eq(Api::AddressNotAvailableReason::Blocked));
    }
    reset();

    // AddressExists
    {
        auto address = Api::Address("exists#example.com");
        auto task = uut->registerAccountAsync(
                    address, boost::none, nullptr, "", kulloForcedNn(registerAccountListener));
        ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
        EXPECT_THAT(registerAccountListener->callback_, Eq(CallbackType::addressNotAvailable));
        EXPECT_THAT(registerAccountListener->address_, Eq(address));
        EXPECT_THAT(registerAccountListener->reason_, Eq(Api::AddressNotAvailableReason::Exists));
    }
    reset();

    // WorksWithoutChallenge
    {
        auto address = Api::Address("nochallenge#example.com");
        auto task = uut->registerAccountAsync(
                    address, boost::none, nullptr, "", kulloForcedNn(registerAccountListener));
        ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
        EXPECT_THAT(registerAccountListener->callback_, Eq(CallbackType::finished));
        EXPECT_THAT(registerAccountListener->address_, Eq(address));
        EXPECT_THAT(registerAccountListener->masterKey_, Ne(boost::none));
    }
    reset();

    // WorksWithChallenge
    {
        auto address = Api::Address("withchallenge#example.com");

        // get challenge
        auto task = uut->registerAccountAsync(
                    address, boost::none, nullptr, "", kulloForcedNn(registerAccountListener));
        ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
        EXPECT_THAT(registerAccountListener->callback_, Eq(CallbackType::challengeNeeded));
        EXPECT_THAT(registerAccountListener->address_, Eq(address));
        EXPECT_THAT(registerAccountListener->challenge_, Not(IsNull()));
        auto challenge = registerAccountListener->challenge_;

        reset();

        // register account
        task = uut->registerAccountAsync(
                    address, boost::none, challenge, "42", kulloForcedNn(registerAccountListener));
        ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
        EXPECT_THAT(registerAccountListener->callback_, Eq(CallbackType::finished));
        EXPECT_THAT(registerAccountListener->address_, Eq(address));
        EXPECT_THAT(registerAccountListener->masterKey_, Ne(boost::none));
    }
}
