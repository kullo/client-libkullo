/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "tests/api/apimodeltest.h"

#include <fstream>

#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/api/DraftAttachmentsAddListener.h>
#include <kulloclient/api/DraftAttachmentsContentListener.h>
#include <kulloclient/api/DraftAttachmentsSaveToListener.h>
#include <kulloclient/api/LocalError.h>
#include <kulloclient/api_impl/debug.h>
#include <kulloclient/api_impl/sessionimpl.h>
#include <kulloclient/dao/attachmentdao.h>
#include <kulloclient/util/binary.h>

#include "tests/testutil.h"

using namespace testing;
using namespace Kullo;

namespace {
class AddListener : public Api::DraftAttachmentsAddListener
{
public:
    void finished(int64_t convId, int64_t attId, const std::string &path) override
    {
        isFinished_ = true;
        convId_ = convId;
        attId_ = attId;
        path_ = path;
    }

    void error(int64_t convId, const std::string &path, Api::LocalError error) override
    {
        K_UNUSED(convId);
        K_UNUSED(path);
        hasError_ = true;
        errorType_ = error;
    }

    bool isFinished_ = false;
    bool hasError_ = false;
    Api::LocalError errorType_;
    int64_t convId_;
    int64_t attId_;
    std::string path_;
};

class ContentListener : public Api::DraftAttachmentsContentListener
{
public:
    void finished(int64_t convId, int64_t attId, const std::vector<uint8_t> &content) override
    {
        convId_ = convId;
        attId_ = attId;
        content_ = content;
    }

    int64_t convId_;
    int64_t attId_;
    std::vector<uint8_t> content_;
};

class SaveToListener : public Api::DraftAttachmentsSaveToListener
{
public:
    void finished(int64_t convId, int64_t attId, const std::string &path) override
    {
        isFinished_ = true;
        convId_ = convId;
        attId_ = attId;
        path_ = path;
    }

    void error(int64_t convId, int64_t attId, const std::string &path,
               Api::LocalError error) override
    {
        K_UNUSED(convId);
        K_UNUSED(attId);
        K_UNUSED(path);
        hasError_ = true;
        errorType_ = error;
    }

    bool isFinished_ = false;
    bool hasError_ = false;
    Api::LocalError errorType_;
    int64_t convId_;
    int64_t attId_;
    std::string path_;
};
}

class ApiDraftAttachments : public ApiModelTest
{
public:
    ApiDraftAttachments()
    {
        data.convId = 1;
        data.attId = 1337;
        data.filename = "apidraftattachments-in.txt";
        data.mimeType = "text/plain";
        data.content = "Hello, world.";
        data.size = data.content.size();
        // echo -n "Hello, world." | sha512sum
        data.hash = "ad0c37c31d69b315f3a81f13c8cde701"
                    "094ad91725ba1b0dc3199ca9713661b8"
                    "280d6ef7e68f133e6211e2e5a9a31504"
                    "45d76f1708e04521b0ee034f0b0baf26";
        data.filepath = TestUtil::tempPath() + "/apidraftattachments-in.txt";
        std::ofstream(data.filepath) << data.content;

        data.outpath = TestUtil::tempPath() + "/apidraftattachments-out.txt";
        data.errorOutpath = "/some/non-existing/dir/-.-.-.-.-.-/file.txt";
        data.errorFilepathDirectory = TestUtil::tempPath();
        data.errorFilepathNonExisting = TestUtil::tempPath() + "/__a-non-existing-file__";
        data.errorFilepathNonReadable = "/etc/shadow"; // A an existing regular file known to be unreadable

        dbSession_ = Db::makeSession(dbPath_);
        Db::migrate(dbSession_);

        Dao::AttachmentDao dao(dbSession_);
        dao.setDraft(true);
        dao.setMessageId(data.convId);
        dao.setIndex(data.attId);
        dao.setFilename(data.filename);
        dao.setMimeType(data.mimeType);
        dao.setSize(data.size);
        dao.setHash(data.hash);
        dao.save();
        dao.setContent(Util::to_vector(data.content));

        makeSession();
        uut = session_->draftAttachments();
    }

protected:
    struct
    {
        int64_t convId;
        int64_t attId;
        std::string filename;
        std::string mimeType;
        int64_t size;
        std::string hash;
        std::string filepath;
        std::string content;
        std::string outpath;
        std::string errorOutpath;
        std::string errorFilepathDirectory;
        std::string errorFilepathNonExisting;
        std::string errorFilepathNonReadable;
    } data;

    Db::SharedSessionPtr dbSession_;
    std::shared_ptr<Api::DraftAttachments> uut;
};

K_TEST_F(ApiDraftAttachments, allForDraftWorks)
{
    auto attachments = uut->allForDraft(data.convId);
    ASSERT_THAT(attachments.size(), Eq(1u));
    EXPECT_THAT(attachments[0], Eq(data.attId));

    EXPECT_THAT(uut->allForDraft(42), IsEmpty());
}

K_TEST_F(ApiDraftAttachments, addAsyncFailsOnNull)
{
    EXPECT_THROW(uut->addAsync(
                     data.convId, data.filepath, data.mimeType, nullptr),
                 Util::AssertionFailed);
}

K_TEST_F(ApiDraftAttachments, addAsyncCanBeCanceled)
{
    auto listener = std::make_shared<AddListener>();
    auto task = uut->addAsync(
                data.convId, data.filepath, data.mimeType, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiDraftAttachments, addAsyncWorks)
{
    auto listener = std::make_shared<AddListener>();
    auto task = uut->addAsync(
                data.convId, data.filepath, data.mimeType, listener);

    ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                Eq(TestUtil::OK));
    EXPECT_THAT(listener->convId_, Eq(data.convId));
    EXPECT_THAT(listener->attId_, Ge(0));
    EXPECT_THAT(listener->path_, Eq(data.filepath));

    {
        // Check if data is stored
        auto messageId = listener->convId_;
        auto attachmentIndex = listener->attId_;
        auto dao = Dao::AttachmentDao::load(Dao::IsDraft::Yes, messageId, attachmentIndex, dbSession_);
        ASSERT_THAT(dao, NotNull());
        EXPECT_THAT(dao->filename(), Eq(data.filename));
        EXPECT_THAT(dao->mimeType(), Eq(data.mimeType));
        EXPECT_THAT(dao->size(), Eq(static_cast<uint64_t>(data.size)));
        EXPECT_THAT(dao->hash(), Eq(data.hash));
        EXPECT_THAT(dao->content(), Eq(Util::to_vector(data.content)));
    }
}

K_TEST_F(ApiDraftAttachments, addAsyncWorksOnError)
{
    suppressErrorLogOutput();

    // Add directory
    {
        auto listener = std::make_shared<AddListener>();
        auto task = uut->addAsync(data.convId, data.errorFilepathDirectory,
                                  data.mimeType, listener);

        ASSERT_THAT(TestUtil::waitAndCheck(task, listener->hasError_),
                    Eq(TestUtil::OK));
        EXPECT_THAT(listener->errorType_, Eq(Api::LocalError::Filesystem));
    }

    // Add Non-existing file
    {
        auto listener = std::make_shared<AddListener>();
        auto task = uut->addAsync(data.convId, data.errorFilepathNonExisting,
                                  data.mimeType, listener);

        ASSERT_THAT(TestUtil::waitAndCheck(task, listener->hasError_),
                    Eq(TestUtil::OK));
        EXPECT_THAT(listener->errorType_, Eq(Api::LocalError::Filesystem));
    }

    // Add non-readable file
    {
        auto listener = std::make_shared<AddListener>();
        auto task = uut->addAsync(data.convId, data.errorFilepathNonReadable,
                                  data.mimeType, listener);

        ASSERT_THAT(TestUtil::waitAndCheck(task, listener->hasError_),
                    Eq(TestUtil::OK));
        EXPECT_THAT(listener->errorType_, Eq(Api::LocalError::Filesystem));
    }
}

K_TEST_F(ApiDraftAttachments, removeWorks)
{
    EXPECT_NO_THROW(uut->remove(data.convId, data.attId));
    EXPECT_THAT(uut->allForDraft(data.convId), IsEmpty());

    EXPECT_NO_THROW(uut->remove(42, 0));
}

K_TEST_F(ApiDraftAttachments, removeWorksExtended)
{
    int64_t conversationId = 588; // test specific to avoid using default generated dao
    int64_t attachmentId1 = 0;
    int64_t attachmentId2 = 100;
    int64_t attachmentId3 = 200000;
    int64_t attachmentId4 = Kullo::ID_MAX;

    // attachment doesn't exist
    ASSERT_THAT(uut->allForDraft(conversationId).size(), Eq(0u));

    // Action 1: insert four attachments
    {
        Dao::AttachmentDao dao(dbSession_);
        dao.setDraft(true);
        dao.setMessageId(conversationId);
        dao.setIndex(attachmentId1);
        dao.setFilename(data.filename);
        dao.setMimeType(data.mimeType);
        dao.setSize(data.size);
        dao.setHash(data.hash);
        dao.save();
        dao.setContent(Util::to_vector(data.content));
    }
    {
        Dao::AttachmentDao dao(dbSession_);
        dao.setDraft(true);
        dao.setMessageId(conversationId);
        dao.setIndex(attachmentId2);
        dao.setFilename(data.filename);
        dao.setMimeType(data.mimeType);
        dao.setSize(data.size);
        dao.setHash(data.hash);
        dao.save();
        dao.setContent(Util::to_vector(data.content));
    }
    {
        Dao::AttachmentDao dao(dbSession_);
        dao.setDraft(true);
        dao.setMessageId(conversationId);
        dao.setIndex(attachmentId3);
        dao.setFilename(data.filename);
        dao.setMimeType(data.mimeType);
        dao.setSize(data.size);
        dao.setHash(data.hash);
        dao.save();
        dao.setContent(Util::to_vector(data.content));
    }
    {
        Dao::AttachmentDao dao(dbSession_);
        dao.setDraft(true);
        dao.setMessageId(conversationId);
        dao.setIndex(attachmentId4);
        dao.setFilename(data.filename);
        dao.setMimeType(data.mimeType);
        dao.setSize(data.size);
        dao.setHash(data.hash);
        dao.save();
        dao.setContent(Util::to_vector(data.content));
    }

    // notify draftAttachmentAdded
    {
        auto listener = std::dynamic_pointer_cast<Event::DraftAttachmentsEventListener>(
                    uut);
        ASSERT_THAT(listener, Not(IsNull()));
        listener->draftAttachmentAdded(Data::DraftAttachment(conversationId, attachmentId1));
        listener->draftAttachmentAdded(Data::DraftAttachment(conversationId, attachmentId2));
        listener->draftAttachmentAdded(Data::DraftAttachment(conversationId, attachmentId3));
        listener->draftAttachmentAdded(Data::DraftAttachment(conversationId, attachmentId4));
    }

    // Status check 1: all four attachments exist
    ASSERT_THAT(uut->allForDraft(conversationId).size(), Eq(4u));
    EXPECT_THAT(uut->allForDraft(conversationId)[0], Eq(attachmentId1));
    EXPECT_THAT(uut->allForDraft(conversationId)[1], Eq(attachmentId2));
    EXPECT_THAT(uut->allForDraft(conversationId)[2], Eq(attachmentId3));
    EXPECT_THAT(uut->allForDraft(conversationId)[3], Eq(attachmentId4));

    // Action 2: Remove from middle
    uut->remove(conversationId, attachmentId3);

    // Event check 2
    {
        auto events = sessionListener_->externalEvents();
        EXPECT_THAT(events, Contains(Api::Event(
                                         Api::EventType::DraftAttachmentRemoved,
                                         conversationId, -1, attachmentId3)
                                     ));
    }

    // Status check 2: three attachments left, order unchanged
    ASSERT_THAT(uut->allForDraft(conversationId).size(), Eq(3u));
    EXPECT_THAT(uut->allForDraft(conversationId)[0], Eq(attachmentId1));
    EXPECT_THAT(uut->allForDraft(conversationId)[1], Eq(attachmentId2));
    EXPECT_THAT(uut->allForDraft(conversationId)[2], Eq(attachmentId4));

    // Action 3: Remove from beginning
    uut->remove(conversationId, attachmentId1);

    // Event check 3
    {
        auto events = sessionListener_->externalEvents();
        EXPECT_THAT(events, Contains(Api::Event(
                                         Api::EventType::DraftAttachmentRemoved,
                                         conversationId, -1, attachmentId1)
                                     ));
    }

    // Status check 3: two attachments left, order unchanged
    ASSERT_THAT(uut->allForDraft(conversationId).size(), Eq(2u));
    EXPECT_THAT(uut->allForDraft(conversationId)[0], Eq(attachmentId2));
    EXPECT_THAT(uut->allForDraft(conversationId)[1], Eq(attachmentId4));

    // Action 4: Remove from end
    uut->remove(conversationId, attachmentId4);

    // Event check 4
    {
        auto events = sessionListener_->externalEvents();
        EXPECT_THAT(events, Contains(Api::Event(
                                         Api::EventType::DraftAttachmentRemoved,
                                         conversationId, -1, attachmentId4)
                                     ));
    }

    // Status check 4: two attachments left, order unchanged
    ASSERT_THAT(uut->allForDraft(conversationId).size(), Eq(1u));
    EXPECT_THAT(uut->allForDraft(conversationId)[0], Eq(attachmentId2));

    // Action 5: Remove last element
    uut->remove(conversationId, attachmentId2);

    // Event check 5
    {
        auto events = sessionListener_->externalEvents();
        EXPECT_THAT(events, Contains(Api::Event(
                                         Api::EventType::DraftAttachmentRemoved,
                                         conversationId, -1, attachmentId2)
                                     ));
    }

    // Status check 5: no attachments left
    ASSERT_THAT(uut->allForDraft(conversationId).size(), Eq(0u));
}

K_TEST_F(ApiDraftAttachments, filenameWorks)
{
    EXPECT_THAT(uut->filename(data.convId, data.attId), StrEq(data.filename));

    EXPECT_THAT(uut->filename(42, 0), StrEq(""));
}

K_TEST_F(ApiDraftAttachments, setFilenameChangesExistingAttachment)
{
    std::string newName = "new_name.txt";
    EXPECT_THAT(uut->filename(data.convId, data.attId), StrEq(data.filename));
    uut->setFilename(data.convId, data.attId, newName);
    EXPECT_THAT(uut->filename(data.convId, data.attId), StrEq(newName));
}

K_TEST_F(ApiDraftAttachments, setFilenameDoesNothingOnNonexistingAttachment)
{
    EXPECT_THAT(uut->filename(42, 0), StrEq(""));
    uut->setFilename(42, 0, "new_name.txt");
    EXPECT_THAT(uut->filename(42, 0), StrEq(""));
}

K_TEST_F(ApiDraftAttachments, mimeTypeWorks)
{
    EXPECT_THAT(uut->mimeType(data.convId, data.attId), StrEq(data.mimeType));

    EXPECT_THAT(uut->mimeType(42, 0), StrEq(""));
}

K_TEST_F(ApiDraftAttachments, sizeWorks)
{
    EXPECT_THAT(uut->size(data.convId, data.attId), Eq(data.size));

    EXPECT_THAT(uut->size(42, 0), 0);
}

K_TEST_F(ApiDraftAttachments, hashWorks)
{
    EXPECT_THAT(uut->hash(data.convId, data.attId), StrEq(data.hash));

    EXPECT_THAT(uut->hash(42, 0), StrEq(""));
}

K_TEST_F(ApiDraftAttachments, contentAsyncFailsOnNull)
{
    EXPECT_THROW(uut->contentAsync(data.convId, data.attId, nullptr),
                 Util::AssertionFailed);
}

K_TEST_F(ApiDraftAttachments, contentAsyncCanBeCanceled)
{
    auto listener = std::make_shared<ContentListener>();
    auto task = uut->contentAsync(data.convId, data.attId, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiDraftAttachments, contentAsyncWorks)
{
    auto listener = std::make_shared<ContentListener>();
    auto task = uut->contentAsync(data.convId, data.attId, listener);

    ASSERT_THAT(task->waitForMs(TestUtil::asyncTimeoutMs()), Eq(true));
    EXPECT_THAT(listener->convId_, Eq(data.convId));
    EXPECT_THAT(listener->attId_, Eq(data.attId));
    EXPECT_THAT(listener->content_, Eq(Util::to_vector(data.content)));
}

K_TEST_F(ApiDraftAttachments, saveToAsyncFailsOnNull)
{
    auto listener = std::make_shared<SaveToListener>();
    EXPECT_THROW(uut->saveToAsync(data.convId, data.attId, "", nullptr),
                 Util::AssertionFailed);
    EXPECT_THROW(uut->saveToAsync(data.convId, data.attId, "", listener),
                 Util::AssertionFailed);
    EXPECT_THROW(uut->saveToAsync(data.convId, data.attId, data.outpath, nullptr),
                 Util::AssertionFailed);
}

K_TEST_F(ApiDraftAttachments, saveToAsyncCanBeCanceled)
{
    auto listener = std::make_shared<SaveToListener>();
    auto task = uut->saveToAsync(data.convId, data.attId, data.outpath, listener);
    ASSERT_THAT(task, Not(IsNull()));
    EXPECT_NO_THROW(task->cancel());
}

K_TEST_F(ApiDraftAttachments, saveToAsyncWorks)
{
    auto listener = std::make_shared<SaveToListener>();
    auto task = uut->saveToAsync(data.convId, data.attId, data.outpath, listener);

    ASSERT_THAT(TestUtil::waitAndCheck(task, listener->isFinished_),
                Eq(TestUtil::OK));
    EXPECT_THAT(listener->convId_, Eq(data.convId));
    EXPECT_THAT(listener->attId_, Eq(data.attId));
    EXPECT_THAT(listener->path_, Eq(data.outpath));

    std::ifstream stream(data.outpath);
    std::stringstream result;
    result << stream.rdbuf();
    EXPECT_THAT(result.str(), Eq(data.content));
}

K_TEST_F(ApiDraftAttachments, saveToAsyncWorksOnError)
{
    suppressErrorLogOutput();

    // Save to non-existing directory
    {
        auto listener = std::make_shared<SaveToListener>();
        auto task = uut->saveToAsync(data.convId, data.attId, data.errorOutpath, listener);

        ASSERT_THAT(TestUtil::waitAndCheck(task, listener->hasError_),
                    Eq(TestUtil::OK));
        EXPECT_THAT(listener->errorType_, Eq(Api::LocalError::Filesystem));
    }
}

K_TEST_F(ApiDraftAttachments, draftAttachmentAddedAndDraftRemoved)
{
    // attachment doesn't exist
    ASSERT_THAT(uut->allForDraft(42).size(), Eq(0u));

    // insert AttachmentDao
    Dao::AttachmentDao dao(dbSession_);
    dao.setDraft(true);
    dao.setMessageId(42);
    dao.setIndex(0);
    dao.setFilename(data.filename);
    dao.setMimeType(data.mimeType);
    dao.setSize(data.size);
    dao.setHash(data.hash);
    dao.save();
    dao.setContent(Util::to_vector(data.content));

    // notify draftAttachmentAdded
    {
        auto listener =
                std::dynamic_pointer_cast<Event::DraftAttachmentsEventListener>(
                    uut);
        ASSERT_THAT(listener, Not(IsNull()));
        Data::DraftAttachment data(42, 0);
        auto result = listener->draftAttachmentAdded(data);

        // check result
        Event::ApiEvents expectedResult = {
            Api::Event(Api::EventType::DraftAttachmentAdded, 42, -1, 0)
        };
        EXPECT_THAT(result, Eq(expectedResult));
    }

    // attachment does exist now
    ASSERT_THAT(uut->size(42, 0), Eq(data.size));


    // notify draftRemoved
    {
        auto listener =
                std::dynamic_pointer_cast<Event::RemovalEventListener>(
                    uut);
        ASSERT_THAT(listener, Not(IsNull()));
        auto result = listener->draftRemoved(42);

        // check result
        Event::ApiEvents expectedResult = {
            Api::Event(Api::EventType::DraftAttachmentRemoved, 42, -1, 0)
        };
        EXPECT_THAT(result, Eq(expectedResult));
    }

    // attachment doesn't exist anymore
    ASSERT_THAT(uut->allForDraft(42).size(), Eq(0u));
}

K_TEST_F(ApiDraftAttachments, idRangeWorks)
{
    auto addAsyncListener = std::make_shared<AddListener>();
    auto contentListener = std::make_shared<ContentListener>();
    auto saveToListener = std::make_shared<SaveToListener>();

    for (auto convId : TEST_IDS_VALID)
    {
        EXPECT_NO_THROW(uut->allForDraft(convId));
        EXPECT_NO_THROW(uut->addAsync(convId, data.filepath, data.mimeType, addAsyncListener));

        for (auto attId : TEST_IDS_VALID)
        {
            EXPECT_NO_THROW(uut->remove(convId, attId));
            EXPECT_NO_THROW(uut->filename(convId, attId));
            EXPECT_NO_THROW(uut->setFilename(convId, attId, data.filename));
            EXPECT_NO_THROW(uut->mimeType(convId, attId));
            EXPECT_NO_THROW(uut->size(convId, attId));
            EXPECT_NO_THROW(uut->hash(convId, attId));
            EXPECT_NO_THROW(uut->contentAsync(convId, attId, contentListener));
            EXPECT_NO_THROW(uut->saveToAsync(convId, attId, data.outpath, saveToListener));
        }

        for (auto attId : TEST_IDS_INVALID)
        {
            EXPECT_ANY_THROW(uut->remove(convId, attId));
            EXPECT_ANY_THROW(uut->filename(convId, attId));
            EXPECT_ANY_THROW(uut->setFilename(convId, attId, data.filename));
            EXPECT_ANY_THROW(uut->mimeType(convId, attId));
            EXPECT_ANY_THROW(uut->size(convId, attId));
            EXPECT_ANY_THROW(uut->hash(convId, attId));
            EXPECT_ANY_THROW(uut->contentAsync(convId, attId, contentListener));
            EXPECT_ANY_THROW(uut->saveToAsync(convId, attId, data.outpath, saveToListener));
        }
    }

    for (auto convId : TEST_IDS_INVALID)
    {
        EXPECT_ANY_THROW(uut->allForDraft(convId));
        EXPECT_ANY_THROW(uut->addAsync(convId, data.filepath, data.mimeType, addAsyncListener));

        for (auto attId : TEST_IDS_VALID)
        {
            EXPECT_ANY_THROW(uut->remove(convId, attId));
            EXPECT_ANY_THROW(uut->filename(convId, attId));
            EXPECT_ANY_THROW(uut->setFilename(convId, attId, data.filename));
            EXPECT_ANY_THROW(uut->mimeType(convId, attId));
            EXPECT_ANY_THROW(uut->size(convId, attId));
            EXPECT_ANY_THROW(uut->hash(convId, attId));
            EXPECT_ANY_THROW(uut->contentAsync(convId, attId, contentListener));
            EXPECT_ANY_THROW(uut->saveToAsync(convId, attId, data.outpath, saveToListener));
        }

        for (auto attId : TEST_IDS_INVALID)
        {
            EXPECT_ANY_THROW(uut->remove(convId, attId));
            EXPECT_ANY_THROW(uut->filename(convId, attId));
            EXPECT_ANY_THROW(uut->setFilename(convId, attId, data.filename));
            EXPECT_ANY_THROW(uut->mimeType(convId, attId));
            EXPECT_ANY_THROW(uut->size(convId, attId));
            EXPECT_ANY_THROW(uut->hash(convId, attId));
            EXPECT_ANY_THROW(uut->contentAsync(convId, attId, contentListener));
            EXPECT_ANY_THROW(uut->saveToAsync(convId, attId, data.outpath, saveToListener));
        }
    }
}
