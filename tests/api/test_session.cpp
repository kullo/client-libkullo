/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <boost/optional.hpp>
#include <kulloclient/registry.h>
#include <kulloclient/api/AccountInfo.h>
#include <kulloclient/api/Address.h>
#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/api/MasterKey.h>
#include <kulloclient/api/PushToken.h>
#include <kulloclient/api/SessionAccountInfoListener.h>
#include <kulloclient/api_impl/clientcreatesessionworker.h>
#include <kulloclient/api_impl/sessionimpl.h>
#include <kulloclient/util/assert.h>

#include "tests/http/fake_httpclientfactory.h"
#include "tests/event/mock_event.h"
#include "tests/testdata.h"
#include "tests/testutil.h"

using namespace testing;
using namespace Kullo;

namespace {
class AccountInfoListener : public Api::SessionAccountInfoListener
{
public:
    void finished(const Api::AccountInfo &accountInfo) override
    {
        isFinished_ = true;
        accountInfo_ = accountInfo;
    }

    void error(Api::NetworkError error) override
    {
        (void)error;
    }

    bool isFinished_ = false;
    boost::optional<Api::AccountInfo> accountInfo_;
};
}

class ApiSession : public ApiModelTest
{
public:
    ApiSession()
    {
        auto httpClientFactory = std::make_shared<FakeHttpClientFactory>();
        Registry::setHttpClientFactory(httpClientFactory);

        makeSession();
        uut = session_;
    }

protected:
    void testRegisterPushTokenWaiting(const std::string &token)
    {
        auto task = uut->registerPushToken(makeGcmAndroidToken(token));
        // wait for 11 seconds because the retry interval is 10 seconds
        ASSERT_THAT(task->waitForMs(11*1000), Eq(false));
        task->cancel();
        ASSERT_THAT(task->waitForMs(2000), Eq(true));
    }

    Api::PushToken makeGcmAndroidToken(const std::string &token)
    {
        return Api::PushToken(
                    Api::PushTokenType::GCM,
                    token,
                    Api::PushTokenEnvironment::Android);
    }

    std::shared_ptr<Api::Session> uut;
};

K_TEST_F(ApiSession, userSettingsWorks)
{
    auto result = uut->userSettings();
    ASSERT_THAT(result, Not(IsNull()));
    EXPECT_THAT(result->address()->isEqualTo(address_), Eq(true));
    EXPECT_THAT(result->masterKey()->isEqualTo(masterKey_), Eq(true));
}

K_TEST_F(ApiSession, conversationsWorks)
{
    EXPECT_THAT(uut->conversations(), Not(IsNull()));
}

K_TEST_F(ApiSession, messagesWorks)
{
    EXPECT_THAT(uut->messages(), Not(IsNull()));
}

K_TEST_F(ApiSession, messageAttachmentsWorks)
{
    EXPECT_THAT(uut->messageAttachments(), Not(IsNull()));
}

K_TEST_F(ApiSession, sendersWorks)
{
    EXPECT_THAT(uut->senders(), Not(IsNull()));
}

K_TEST_F(ApiSession, draftsWorks)
{
    EXPECT_THAT(uut->drafts(), Not(IsNull()));
}

K_TEST_F(ApiSession, draftAttachmentsWorks)
{
    EXPECT_THAT(uut->draftAttachments(), Not(IsNull()));
}

K_TEST_F(ApiSession, syncerWorks)
{
    EXPECT_THAT(uut->syncer(), Not(IsNull()));
}

K_TEST_F(ApiSession, accountInfoAsyncFailsOnNull)
{
    EXPECT_THROW(uut->accountInfoAsync(nullptr), Util::AssertionFailed);
}

K_TEST_F(ApiSession, accountInfoAsyncCanBeCanceled)
{
    auto listener = std::make_shared<AccountInfoListener>();
    auto task = uut->accountInfoAsync(listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiSession, accountInfoAsyncWorks)
{
    auto listener = std::make_shared<AccountInfoListener>();
    auto task = uut->accountInfoAsync(listener);

    ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                Eq(TestUtil::OK));
    EXPECT_THAT(listener->accountInfo_->settingsUrl,
                StrEq("https://accounts.example.com/foo/bar"));
}

K_TEST_F(ApiSession, registerPushTokenFailsOnEmpty)
{
    EXPECT_THROW(uut->registerPushToken(makeGcmAndroidToken("")),
                 Util::AssertionFailed);
}

K_TEST_F(ApiSession, registerPushTokenCanBeCanceled)
{
    auto task = uut->registerPushToken(makeGcmAndroidToken("token_123_token"));
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiSession, registerPushTokenWorks)
{
    auto task = uut->registerPushToken(makeGcmAndroidToken("token_123_token"));
    task->waitUntilDone();
}

K_TEST_F(ApiSession, registerPushTokenReturnsEarlyOnClientError)
{
    suppressErrorLogOutput();

    auto task = uut->registerPushToken(makeGcmAndroidToken("return_http_400"));
    ASSERT_THAT(task->waitForMs(1000), Eq(true));
}

K_TEST_F(ApiSession, DISABLED_registerPushTokenRetriesOnConnectionError)
{
    testRegisterPushTokenWaiting("return_connection_error");
}

K_TEST_F(ApiSession, DISABLED_registerPushTokenRetriesOnTimeout)
{
    testRegisterPushTokenWaiting("return_timeout");
}

K_TEST_F(ApiSession, DISABLED_registerPushTokenRetriesOnServerError)
{
    testRegisterPushTokenWaiting("return_http_500");
}

K_TEST_F(ApiSession, notifyWorks)
{
    Api::Event apiEvent(Api::EventType::ConversationAdded, 1, 2,3);
    Event::ApiEvents apiEventSet = { apiEvent };
    std::vector<Api::Event> apiEventVector = { apiEvent };

    auto event = std::make_shared<MockEvent>();
    EXPECT_CALL(*event, notify(_))
            .Times(1)
            .WillRepeatedly(Return(apiEventSet));

    EXPECT_THAT(uut->notify(event), Eq(apiEventVector));
}
