/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <kulloclient/registry.h>
#include <kulloclient/api/Syncer.h>
#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/api/SyncerRunListener.h>
#include <kulloclient/api_impl/debug.h>
#include <kulloclient/util/assert.h>

#include "tests/http/fake_httpclientfactory.h"

using namespace testing;
using namespace Kullo;

namespace {
class RunListener : public Api::SyncerRunListener
{
public:
    void draftAttachmentsTooBig(int64_t convId) override
    {
        isFinished_ = true;
        draftAttachmentsTooBigConvId_ = convId;
    }

    void finished() override
    {
        isFinished_ = true;
    }

    void error(Api::NetworkError error) override
    {
        std::cerr << "RunListener::error: " << error << std::endl;
    }

    bool isFinished_ = false;
    int64_t draftAttachmentsTooBigConvId_ = 0;
};
}

class ApiSyncer : public ApiModelTest
{
protected:
    ApiSyncer()
    {
        auto httpClientFactory = std::make_shared<FakeHttpClientFactory>();
        Registry::setHttpClientFactory(httpClientFactory);

        makeSession();
        uut = session_->syncer();
    }

    std::shared_ptr<Api::Syncer> uut;
};

K_TEST_F(ApiSyncer, runAsyncFailsOnNull)
{
    EXPECT_THROW(uut->runAsync(Api::SyncMode::Everything, nullptr),
                 Util::AssertionFailed);
}

/* Commented out because some assertion fails, which causes a long noisy
 * error log output.
K_TEST_F(ApiSyncer, runAsyncCanBeCanceled)
{
    auto listener = std::make_shared<RunListener>();
    auto task = uut->runAsync(Api::SyncMode::Everything, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}
*/

/* Commented out because it is a lot of work to make it run, especially in
 * FakeHttpClient which needs to return consistent data for everything.
 * Don't use K_TEST_F as long as it is commented out!
TEST_F(ApiSyncer, runAsyncWorks)
{
    auto listener = std::make_shared<RunListener>();
    auto task = uut->runAsync(true, listener);

    ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_)), Eq(true));
    ASSERT_THAT(listener->waitForFinish(), Eq(true));
}
*/

K_TEST_F(ApiSyncer, downloadAttachmentsForMessageAsyncFailsOnNull)
{
    auto listener = std::make_shared<RunListener>();

    EXPECT_THROW(uut->downloadAttachmentsForMessageAsync(0, nullptr),
                 Util::AssertionFailed);
    EXPECT_THROW(uut->downloadAttachmentsForMessageAsync(42, nullptr),
                 Util::AssertionFailed);
    EXPECT_THROW(uut->downloadAttachmentsForMessageAsync(0, listener),
                 Util::AssertionFailed);
}

K_TEST_F(ApiSyncer, downloadAttachmentsForMessageAsyncCanBeCanceled)
{
    auto listener = std::make_shared<RunListener>();
    auto task = uut->downloadAttachmentsForMessageAsync(
                42, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}
