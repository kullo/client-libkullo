/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <kulloclient/registry.h>
#include <kulloclient/api/Syncer.h>
#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/api/SyncerListener.h>
#include <kulloclient/api_impl/debug.h>
#include <kulloclient/util/assert.h>

#include "tests/http/fake_httpclientfactory.h"
#include "tests/testutil.h"

using namespace testing;
using namespace Kullo;

namespace {
class CountingSyncListener : public Api::SyncerListener
{
public:
    void draftAttachmentsTooBig(int64_t convId) override
    {
        K_UNUSED(convId);
    }
    void progressed(const Api::SyncProgress & progress) override
    {
        K_UNUSED(progress);
        countProgressed_++;
    }
    void finished() override
    {
        isFinished_ = true;
    }
    void error(Api::NetworkError error) override
    {
        K_UNUSED(error);
    }

    int countProgressed_ = 0;
    bool isFinished_ = false;
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
        uut_ = session_->syncer();
    }

    std::shared_ptr<Api::Syncer> uut_;
};

K_TEST_F(ApiSyncer, progressWorks)
{
    auto listener = std::make_shared<CountingSyncListener>();
    uut_->setListener(listener);
    uut_->requestSync(Api::SyncMode::WithoutAttachments);

    auto task = uut_->asyncTask();
    ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                Eq(TestUtil::OK));

    EXPECT_THAT(listener->countProgressed_, Ge(1));
}

K_TEST_F(ApiSyncer, idRangeWorks)
{
    for (auto id : TEST_IDS_VALID)
    {
        EXPECT_NO_THROW(uut_->requestDownloadingAttachmentsForMessage(id));
    }

    for (auto id : TEST_IDS_INVALID)
    {
        EXPECT_THROW(uut_->requestDownloadingAttachmentsForMessage(id), Util::AssertionFailed);
    }
}
