/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/api/MessageAttachments.h>
#include <kulloclient/api/MessageAttachmentsContentListener.h>
#include <kulloclient/api/MessageAttachmentsSaveToListener.h>
#include <kulloclient/api/LocalError.h>
#include <kulloclient/api_impl/debug.h>
#include <kulloclient/api_impl/sessionimpl.h>
#include <kulloclient/dao/attachmentdao.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/filesystem.h>

#include "tests/testutil.h"

using namespace testing;
using namespace Kullo;

namespace {
class ContentListener : public Api::MessageAttachmentsContentListener
{
public:
    void finished(
            int64_t msgId, int64_t attId,
            const std::vector<uint8_t> &content) override
    {
        msgId_ = msgId;
        attId_ = attId;
        content_ = content;
    }

    int64_t msgId_ = 0;
    int64_t attId_ = 0;
    std::vector<uint8_t> content_;
};

class SaveToListener : public Api::MessageAttachmentsSaveToListener
{
public:
    void finished(int64_t msgId, int64_t attId, const std::string &path) override
    {
        isFinished_ = true;
        msgId_ = msgId;
        attId_ = attId;
        path_ = path;
    }

    void error(int64_t msgId, int64_t attId, const std::string &path,
               Api::LocalError error) override
    {
        K_UNUSED(msgId);
        K_UNUSED(attId);
        K_UNUSED(path);
        hasError_ = true;
        errorType_ = error;
    }

    bool isFinished_ = false;
    bool hasError_ = false;
    Api::LocalError errorType_;
    int64_t msgId_ = 0;
    int64_t attId_ = 0;
    std::string path_;
};
}

class ApiMessageAttachments : public ApiModelTest
{
public:
    ApiMessageAttachments()
    {
        data.msgId = 1;
        data.attId = 1337;
        data.filename = "foo.txt";
        data.mimeType = "text/plain";
        data.content = "Hello, world.";
        data.size = data.content.size();
        data.hash =
                "0cf9180a764aba863a67b6d72f0918bc"
                "131c6772642cb2dce5a34f0a702f9470"
                "ddc2bf125c12198b1995c233c34b4afd"
                "346c54a2334c350a948a51b6e8b4e6b6";

        data.outpath = TestUtil::tempPath() + "/apimessageattachments-out.txt";
        data.errorOutpath = "/some/non-existing/dir/-.-.-.-.-.-/file.txt";

        dbSession_ = Db::makeSession(sessionConfig_);
        Db::migrate(dbSession_);

        insertAttachment(data.msgId);

        makeSession();
        uut = session_->messageAttachments().as_nullable();
    }

protected:
    void insertAttachment(int64_t msgId)
    {
        Dao::AttachmentDao dao(dbSession_);
        dao.setDraft(false);
        dao.setMessageId(msgId);
        dao.setIndex(data.attId);
        dao.setFilename(data.filename);
        dao.setMimeType(data.mimeType);
        dao.setSize(data.size);
        dao.setHash(data.hash);
        dao.save();
    }

    void setContent()
    {
        auto dao = Dao::AttachmentDao::load(
                    Dao::IsDraft::No, data.msgId, data.attId, dbSession_);
        kulloAssert(dao);
        dao->setContent(Util::to_vector(data.content));
    }

    struct
    {
        int64_t msgId;
        int64_t attId;
        std::string filename;
        std::string mimeType;
        int64_t size;
        std::string hash;
        std::string filepath;
        std::string content;
        std::string outpath;
        std::string errorOutpath;
    } data;

    Db::SharedSessionPtr dbSession_;
    std::shared_ptr<Api::MessageAttachments> uut;
};

K_TEST_F(ApiMessageAttachments, allForMessageWorks)
{
    auto attachments = uut->allForMessage(data.msgId);
    ASSERT_THAT(attachments.size(), Eq(1u));
    EXPECT_THAT(attachments[0], Eq(data.attId));

    EXPECT_THAT(uut->allForMessage(42), IsEmpty());
}

K_TEST_F(ApiMessageAttachments, allAttachmentsDownloadedWorks)
{
    EXPECT_THAT(uut->allAttachmentsDownloaded(data.msgId), Eq(false));
    setContent();
    EXPECT_THAT(uut->allAttachmentsDownloaded(data.msgId), Eq(true));

    EXPECT_THAT(uut->allAttachmentsDownloaded(42), Eq(true));
}

K_TEST_F(ApiMessageAttachments, filenameWorks)
{
    EXPECT_THAT(uut->filename(data.msgId, data.attId), StrEq(data.filename));

    EXPECT_THAT(uut->filename(42, 0), StrEq(""));
}

K_TEST_F(ApiMessageAttachments, mimeTypeWorks)
{
    EXPECT_THAT(uut->mimeType(data.msgId, data.attId), StrEq(data.mimeType));

    EXPECT_THAT(uut->mimeType(42, 0), StrEq(""));
}

K_TEST_F(ApiMessageAttachments, sizeWorks)
{
    EXPECT_THAT(uut->size(data.msgId, data.attId), Eq(data.size));

    EXPECT_THAT(uut->size(42, 0), 0);
}

K_TEST_F(ApiMessageAttachments, hashWorks)
{
    EXPECT_THAT(uut->hash(data.msgId, data.attId), StrEq(data.hash));

    EXPECT_THAT(uut->hash(42, 0), StrEq(""));
}

K_TEST_F(ApiMessageAttachments, contentAsyncCanBeCanceled)
{
    auto listener = Kullo::nn_make_shared<ContentListener>();
    auto task = uut->contentAsync(data.msgId, data.attId, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiMessageAttachments, contentAsyncWorks)
{
    setContent();

    auto listener = Kullo::nn_make_shared<ContentListener>();
    auto task = uut->contentAsync(data.msgId, data.attId, listener);

    ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
    EXPECT_THAT(listener->msgId_, Eq(data.msgId));
    EXPECT_THAT(listener->attId_, Eq(data.attId));
    EXPECT_THAT(listener->content_, Eq(Util::to_vector(data.content)));
}

K_TEST_F(ApiMessageAttachments, saveToAsyncFailsOnNull)
{
    auto listener = Kullo::nn_make_shared<SaveToListener>();
    EXPECT_THROW(uut->saveToAsync(data.msgId, data.attId, "", listener),
                 Util::AssertionFailed);
}

K_TEST_F(ApiMessageAttachments, saveToAsyncCanBeCanceled)
{
    auto listener = Kullo::nn_make_shared<SaveToListener>();
    auto task = uut->saveToAsync(data.msgId, data.attId, data.outpath, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiMessageAttachments, saveToAsyncWorks)
{
    setContent();

    auto listener = Kullo::nn_make_shared<SaveToListener>();
    auto task = uut->saveToAsync(data.msgId, data.attId, data.outpath, listener);

    ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                Eq(TestUtil::OK));
    EXPECT_THAT(listener->msgId_, Eq(data.msgId));
    EXPECT_THAT(listener->attId_, Eq(data.attId));
    EXPECT_THAT(listener->path_, Eq(data.outpath));

    auto stream = Util::Filesystem::makeIfstream(data.outpath);
    std::stringstream result;
    result << stream->rdbuf();
    EXPECT_THAT(result.str(), Eq(data.content));
}

K_TEST_F(ApiMessageAttachments, saveToAsyncWorksOnError)
{
    setContent();
    suppressErrorLogOutput();

    // Save to non-existing directory
    {
        auto listener = Kullo::nn_make_shared<SaveToListener>();
        auto task = uut->saveToAsync(data.msgId, data.attId, data.errorOutpath, listener);

        ASSERT_THAT(TestUtil::waitAndCheck(task, listener->hasError_),
                    Eq(TestUtil::OK));
        EXPECT_THAT(listener->errorType_, Eq(Api::LocalError::Filesystem));
    }
}

K_TEST_F(ApiMessageAttachments, messageAdded)
{
    auto msgIdNewAttachment = data.msgId + 1;
    insertAttachment(msgIdNewAttachment);

    // attachment doesn't exist
    ASSERT_THAT(uut->size(msgIdNewAttachment, data.attId), Eq(0));

    // notify messageAdded
    {
        auto listener =
                std::dynamic_pointer_cast<Event::MessagesEventListener>(uut);
        ASSERT_THAT(listener, Not(IsNull()));
        EXPECT_THAT(listener->messageAdded(0, msgIdNewAttachment), IsEmpty());
    }

    // attachment does now exist
    ASSERT_THAT(uut->size(msgIdNewAttachment, data.attId), Eq(data.size));
}

K_TEST_F(ApiMessageAttachments, messageRemoved)
{
    // attachment does exist
    ASSERT_THAT(uut->size(data.msgId, data.attId), Eq(data.size));

    // notify messageRemoved
    {
        auto listener =
                std::dynamic_pointer_cast<Event::RemovalEventListener>(uut);
        ASSERT_THAT(listener, Not(IsNull()));
        EXPECT_THAT(listener->messageRemoved(0, data.msgId), IsEmpty());
    }

    // attachment doesn't exist anymore
    ASSERT_THAT(uut->size(data.msgId, data.attId), Eq(0));
}

K_TEST_F(ApiMessageAttachments, DISABLED_idRangeWorks)
{
    auto contentListener = Kullo::nn_make_shared<ContentListener>();
    auto saveToListener = Kullo::nn_make_shared<SaveToListener>();

    for (auto msgId : TEST_IDS_VALID)
    {
        EXPECT_NO_THROW(uut->allForMessage(msgId));
        EXPECT_NO_THROW(uut->allAttachmentsDownloaded(msgId));

        for (auto attId : TEST_IDS_VALID)
        {
            EXPECT_NO_THROW(uut->filename(msgId, attId));
            EXPECT_NO_THROW(uut->mimeType(msgId, attId));
            EXPECT_NO_THROW(uut->size(msgId, attId));
            EXPECT_NO_THROW(uut->hash(msgId, attId));
            EXPECT_NO_THROW(uut->contentAsync(msgId, attId, contentListener)->waitUntilDone());
            EXPECT_NO_THROW(uut->saveToAsync(msgId, attId, data.outpath, saveToListener)->waitUntilDone());
        }

        for (auto attId : TEST_IDS_INVALID)
        {
            EXPECT_ANY_THROW(uut->filename(msgId, attId));
            EXPECT_ANY_THROW(uut->mimeType(msgId, attId));
            EXPECT_ANY_THROW(uut->size(msgId, attId));
            EXPECT_ANY_THROW(uut->hash(msgId, attId));
            EXPECT_ANY_THROW(uut->contentAsync(msgId, attId, contentListener)->waitUntilDone());
            EXPECT_ANY_THROW(uut->saveToAsync(msgId, attId, data.outpath, saveToListener)->waitUntilDone());
        }
    }

    for (auto msgId : TEST_IDS_INVALID)
    {
        EXPECT_ANY_THROW(uut->allForMessage(msgId));
        EXPECT_ANY_THROW(uut->allAttachmentsDownloaded(msgId));

        for (auto attId : TEST_IDS_VALID)
        {
            EXPECT_ANY_THROW(uut->filename(msgId, attId));
            EXPECT_ANY_THROW(uut->mimeType(msgId, attId));
            EXPECT_ANY_THROW(uut->size(msgId, attId));
            EXPECT_ANY_THROW(uut->hash(msgId, attId));
            EXPECT_ANY_THROW(uut->contentAsync(msgId, attId, contentListener)->waitUntilDone());
            EXPECT_ANY_THROW(uut->saveToAsync(msgId, attId, data.outpath, saveToListener)->waitUntilDone());
        }

        for (auto attId : TEST_IDS_INVALID)
        {
            EXPECT_ANY_THROW(uut->filename(msgId, attId));
            EXPECT_ANY_THROW(uut->mimeType(msgId, attId));
            EXPECT_ANY_THROW(uut->size(msgId, attId));
            EXPECT_ANY_THROW(uut->hash(msgId, attId));
            EXPECT_ANY_THROW(uut->contentAsync(msgId, attId, contentListener)->waitUntilDone());
            EXPECT_ANY_THROW(uut->saveToAsync(msgId, attId, data.outpath, saveToListener)->waitUntilDone());
        }
    }
}
