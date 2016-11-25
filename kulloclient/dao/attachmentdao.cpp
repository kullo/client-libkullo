/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/attachmentdao.h"

#include <sstream>

#include <smartsqlite/scopedsavepoint.h>

#include "kulloclient/dao/daoutil.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Db;
using namespace Kullo::Util;

namespace Kullo {
namespace Dao {

namespace {
// 1024 is the default block size used by SQLite
const size_t STREAM_BUF_SIZE = 1024;
}

AttachmentDao::AttachmentDao(SharedSessionPtr session)
    : session_(session)
{
}

std::unique_ptr<AttachmentDao> AttachmentDao::load(IsDraft draft, id_type messageId, id_type index, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT draft, message_id, idx, filename, mime_type, size, hash, note "
                "FROM attachments "
                "WHERE draft = :draft AND message_id = :message_id AND idx = :idx");
    stmt.bind(":draft", (draft == IsDraft::Yes));
    stmt.bind(":message_id", messageId);
    stmt.bind(":idx", index);

    return AttachmentResult(std::move(stmt), session).next();
}

int64_t AttachmentDao::msgIdForFirstDownloadable(SharedSessionPtr session)
{
    kulloAssert(session);

    // length(X)=NULL iff X=NULL
    // http://stackoverflow.com/questions/26801658/is-length-is-null-equivalent-and-faster-than-is-null-for-blobs
    auto stmt = session->prepare(
                "SELECT a.message_id "
                "FROM attachments a NATURAL JOIN attachments_content ac "
                "WHERE length(ac.content) IS NULL AND a.draft = 0 "
                "ORDER BY a.message_id "
                "LIMIT 1");

    auto iter = stmt.begin();
    if (iter == stmt.end()) return -1;
    return iter->get<int64_t>(0);
}

std::unique_ptr<AttachmentResult> AttachmentDao::allForMessage(IsDraft draft, id_type messageId, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT draft, message_id, idx, filename, mime_type, size, hash, note "
                "FROM attachments "
                "WHERE draft = :draft AND message_id = :message_id "
                "ORDER BY idx ASC");
    stmt.bind(":draft", (draft == IsDraft::Yes));
    stmt.bind(":message_id", messageId);

    return make_unique<AttachmentResult>(std::move(stmt), session);
}

std::size_t AttachmentDao::sizeOfAllForMessage(IsDraft draft, id_type messageId, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT sum(size) "
                "FROM attachments "
                "WHERE draft = :draft AND message_id = :message_id");
    stmt.bind(":draft", (draft == IsDraft::Yes));
    stmt.bind(":message_id", messageId);

    return stmt.execWithSingleResult().get<std::size_t>(0);
}

std::size_t AttachmentDao::sizeOfAllDownloadable(SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT sum(a.size) "
                "FROM attachments a NATURAL JOIN attachments_content ac "
                "WHERE length(ac.content) IS NULL AND a.draft = 0");

    return stmt.execWithSingleResult().get<std::size_t>(0);
}

std::unique_ptr<AttachmentResult> AttachmentDao::all(
        IsDraft draft, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT draft, message_id, idx, filename, mime_type, size, hash, note "
                "FROM attachments "
                "WHERE draft = :draft "
                "ORDER BY message_id, idx ASC");
    stmt.bind(":draft", (draft == IsDraft::Yes));

    return make_unique<AttachmentResult>(std::move(stmt), session);
}

bool AttachmentDao::allAttachmentsDownloaded(
        int64_t msgId, SharedSessionPtr session)
{
    auto stmt = session->prepare(
                "SELECT count(idx) FROM attachments_content "
                "WHERE message_id = :id AND length(content) IS NULL");
    stmt.bind(":id", msgId);

    auto notYetDownloadedAttachments = stmt.execWithSingleResult().get<int>(0);
    return notYetDownloadedAttachments == 0;
}

bool AttachmentDao::save()
{
    if (deleted_ || !dirty_)
    {
        return false;
    }

    std::string sql;
    if (!storedInDb_)
    {
        auto stmt = session_->prepare(
                    "INSERT INTO attachments_content VALUES ("
                    ":draft, :message_id, :index, NULL)");
        stmt.bind(":draft", draft_);
        stmt.bind(":message_id", messageId_);
        stmt.bind(":index", index_);
        stmt.execWithoutResult();

        sql = "INSERT INTO attachments VALUES ("
                ":draft, :message_id, :index, :filename, "
                ":mime_type, :size, :hash, :note)";
    }
    else
    {
        sql = "UPDATE attachments "
                "SET filename = :filename, mime_type = :mime_type, "
                "size = :size, hash = :hash, note = :note "
                "WHERE draft = :draft AND message_id = :message_id AND idx = :index";
    }
    auto stmt = session_->prepare(sql);
    stmt.bind(":draft", draft_);
    stmt.bind(":message_id", messageId_);
    stmt.bind(":index", index_);
    stmt.bind(":filename", filename_);
    stmt.bind(":mime_type", mimeType_);
    stmt.bind(":size", size_);
    stmt.bind(":hash", hash_);
    stmt.bind(":note", note_);
    stmt.execWithoutResult();

    storedInDb_ = true;
    dirty_ = false;
    return true;
}

void AttachmentDao::convertToMessageAttachment(id_type msgId)
{
    kulloAssert(draft_);
    kulloAssert(storedInDb_);
    kulloAssert(!dirty_);

    // save for SQL query before overwriting it
    auto convId = messageId_;

    draft_ = false;
    messageId_ = msgId;

    {
        std::string sql = "UPDATE attachments "
                "SET draft = 0, message_id = :message_id "
                "WHERE draft = 1 AND message_id = :conv_id AND idx = :index ";

        auto stmt = session_->prepare(sql);
        stmt.bind(":message_id", messageId_);
        stmt.bind(":conv_id", convId);
        stmt.bind(":index", index_);
        stmt.execWithoutResult();
    }

    {
        std::string sql = "UPDATE attachments_content "
            "SET draft = 0, message_id = :message_id "
            "WHERE draft = 1 AND message_id = :conv_id AND idx = :index ";
        auto stmt = session_->prepare(sql);
        stmt.bind(":message_id", messageId_);
        stmt.bind(":conv_id", convId);
        stmt.bind(":index", index_);
        stmt.execWithoutResult();
    }
}

bool AttachmentDao::draft() const
{
    return draft_;
}

bool AttachmentDao::setDraft(bool draft)
{
    if (assignAndUpdateDirty(draft_, draft, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

id_type AttachmentDao::messageId() const
{
    return messageId_;
}

bool AttachmentDao::setMessageId(id_type messageId)
{
    if (assignAndUpdateDirty(messageId_, messageId, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

id_type AttachmentDao::index() const
{
    return index_;
}

bool AttachmentDao::setIndex(id_type index)
{
    if (assignAndUpdateDirty(index_, index, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

std::string AttachmentDao::filename() const
{
    return filename_;
}

bool AttachmentDao::setFilename(const std::string &filename)
{
    return assignAndUpdateDirty(filename_, filename, dirty_);
}

std::string AttachmentDao::mimeType() const
{
    return mimeType_;
}

bool AttachmentDao::setMimeType(const std::string &mimeType)
{
    return assignAndUpdateDirty(mimeType_, mimeType, dirty_);
}

std::size_t AttachmentDao::size() const
{
    return size_;
}

bool AttachmentDao::setSize(std::size_t size)
{
    return assignAndUpdateDirty(size_, size, dirty_);
}

std::string AttachmentDao::hash() const
{
    return hash_;
}

bool AttachmentDao::setHash(const std::string &hash)
{
    return assignAndUpdateDirty(hash_, hash, dirty_);
}

std::vector<unsigned char> AttachmentDao::content() const
{
    std::ostringstream ostream;
    saveContent(ostream);
    return Util::to_vector(ostream.str());
}

void AttachmentDao::saveContent(std::ostream &stream) const
{
    std::int64_t rowid;
    bool isNull;
    {
        auto stmt = session_->prepare(
                    "SELECT rowid, length(content) IS NULL AS content_is_null FROM attachments_content "
                    "WHERE draft = :draft AND message_id = :message_id AND idx = :idx");
        stmt.bind(":draft", draft_);
        stmt.bind(":message_id", messageId_);
        stmt.bind(":idx", index_);
        auto row = stmt.execWithSingleResult();
        rowid = row.get<std::int64_t>("rowid");
        isNull = row.get<bool>("content_is_null");
    }

    if (!isNull)
    {
        auto blob = session_->openBlob("main", "attachments_content", "content", rowid,
                                       SmartSqlite::Blob::READONLY);

        char buffer[STREAM_BUF_SIZE];
        size_t bytesReadTotal = 0;
        while (bytesReadTotal < blob.size())
        {
            auto bytesRead = blob.read(buffer, sizeof(buffer), bytesReadTotal);
            kulloAssert(bytesRead > 0);
            bytesReadTotal += bytesRead;
            stream.write(buffer, bytesRead);
            if (!stream)
            {
                throw Util::FilesystemError("Failed to write attachment to stream");
            }
        }
        kulloAssert(bytesReadTotal == blob.size());
    }
}

void AttachmentDao::setContent(const std::vector<unsigned char> &content)
{
    // content can only be stored to an existing record
    kulloAssert(storedInDb_);

    std::istringstream stream(Util::to_string(content));
    setContent(stream);
}

void AttachmentDao::setContent(
        std::istream &input,
        const std::function<void(const unsigned char *data, std::size_t length, const Progress &progress)> &callback)
{
    const std::string savepointName =
            std::string("attachmentdao_") + std::to_string(draft_) +
            "_" + std::to_string(messageId_) + "_" + std::to_string(index_);
    SmartSqlite::ScopedSavepoint sp(session_, savepointName);

    // get rowid
    std::int64_t rowid;
    {
        auto stmt = session_->prepare(
                    "SELECT rowid FROM attachments_content "
                    "WHERE draft = :draft "
                        "AND message_id = :message_id "
                        "AND idx = :idx");
        stmt.bind(":draft", draft_);
        stmt.bind(":message_id", messageId_);
        stmt.bind(":idx", index_);
        rowid = stmt.execWithSingleResult().get<std::int64_t>(0);
    }

    // set size of attachment
    {
        auto stmt = session_->prepare(
                    "UPDATE attachments_content "
                    "SET content = zeroblob(:size) "
                    "WHERE rowid = :rowid");
        stmt.bind(":size", size_);
        stmt.bind(":rowid", rowid);
        stmt.execWithoutResult();
    }

    // stream content to blob
    if (size_)
    {
        auto blob = session_->openBlob("main", "attachments_content",
                                      "content", rowid,
                                      SmartSqlite::Blob::READWRITE);

        unsigned char buffer[STREAM_BUF_SIZE];

        Progress progress;
        progress.bytesProcessed = 0;
        progress.bytesTotal = size_;

        while (input)
        {
            input.read(reinterpret_cast<char*>(buffer), sizeof(buffer));
            const auto bytesReadCount = input.gcount();
            if (progress.bytesProcessed + bytesReadCount > size_)
            {
                throw Db::DatabaseIntegrityError(
                            "AttachmentDao::setContent: The stored size is "
                            "smaller than the stream size");
            }

            blob.write(buffer, input.gcount(), progress.bytesProcessed /* offset */);
            progress.bytesProcessed += bytesReadCount;

            callback(&buffer[0], bytesReadCount, progress);
        }
        if (input.bad())
        {
            throw Util::FilesystemError("Failed to read attachment from stream");
        }
        if (progress.bytesProcessed < size_)
        {
            throw Db::DatabaseIntegrityError(
                        "AttachmentDao::setContent: The stored size is larger "
                        "than the stream size");
        }
    }

    sp.release();
}

std::string AttachmentDao::note() const
{
    return note_;
}

bool AttachmentDao::setNote(const std::string &note)
{
    return assignAndUpdateDirty(note_, note, dirty_);
}

bool AttachmentDao::deleted() const
{
    return deleted_;
}

void AttachmentDao::deletePermanently()
{
    if (deleted_) return;

    auto stmt = session_->prepare(
                "DELETE FROM attachments "
                "WHERE draft = :draft AND message_id = :message_id AND idx = :idx");
    stmt.bind(":draft", draft_);
    stmt.bind(":message_id", messageId_);
    stmt.bind(":idx", index_);
    stmt.execWithoutResult();

    stmt = session_->prepare(
                "DELETE FROM attachments_content "
                "WHERE draft = :draft AND message_id = :message_id AND idx = :idx");
    stmt.bind(":draft", draft_);
    stmt.bind(":message_id", messageId_);
    stmt.bind(":idx", index_);
    stmt.execWithoutResult();

    deleted_ = true;
    storedInDb_ = false;
    dirty_ = true;
}

id_type AttachmentDao::idForNewDraftAttachment(id_type conversationId, SharedSessionPtr session)
{
    kulloAssert(session);

    // return either the maximum index + 1 or, if none exists, 0
	// Note: {null} + 1 = {null}
    auto stmt = session->prepare(
                "SELECT coalesce(max(idx) + 1, 0) FROM attachments "
                "WHERE draft = 1 AND message_id = :conversation_id");
    stmt.bind(":conversation_id", conversationId);

    return stmt.execWithSingleResult().get<id_type>(0);
}

std::vector<id_type> AttachmentDao::deleteAttachmentsForDraft(id_type conversationId, SharedSessionPtr session)
{
    kulloAssert(session);
    return deleteAttachments(IsDraft::Yes, conversationId, session);
}

std::vector<id_type> AttachmentDao::deleteAttachmentsForMessage(id_type messageId, SharedSessionPtr session)
{
    kulloAssert(session);
    return deleteAttachments(IsDraft::No, messageId, session);
}

std::vector<id_type> AttachmentDao::deleteAttachments(IsDraft draft, id_type messageId, SharedSessionPtr session)
{
    kulloAssert(session);

    const std::string savepointName =
            std::string("attachmentdao_delete_all_")
            + std::to_string(draft == IsDraft::Yes)
            + "_" + std::to_string(messageId);
    SmartSqlite::ScopedSavepoint sp(session, savepointName);

    auto stmt = session->prepare(
                "SELECT idx FROM attachments "
                "WHERE draft = :draft AND message_id = :message_id "
                "ORDER BY idx ASC");
    stmt.bind(":draft", (draft == IsDraft::Yes));
    stmt.bind(":message_id", messageId);

    std::vector<id_type> result;
    for (auto row : stmt)
    {
        result.emplace_back(row.get<id_type>("idx"));
    }

    stmt = session->prepare(
                "DELETE FROM attachments "
                "WHERE draft = :draft AND message_id = :message_id");
    stmt.bind(":draft", (draft == IsDraft::Yes));
    stmt.bind(":message_id", messageId);
    stmt.execWithoutResult();

    stmt = session->prepare(
                "DELETE FROM attachments_content "
                "WHERE draft = :draft AND message_id = :message_id");
    stmt.bind(":draft", (draft == IsDraft::Yes));
    stmt.bind(":message_id", messageId);
    stmt.execWithoutResult();

    sp.release();
    return result;
}

std::unique_ptr<AttachmentDao> AttachmentDao::loadFromDb(const SmartSqlite::Row &row, SharedSessionPtr session)
{
    auto dao = make_unique<AttachmentDao>(session);
    dao->draft_ = row.get<bool>("draft");
    dao->messageId_ = row.get<id_type>("message_id");
    dao->index_ = row.get<std::uint32_t>("idx");
    dao->filename_ = row.get<std::string>("filename");
    dao->mimeType_ = row.get<std::string>("mime_type");
    dao->size_ = row.get<std::size_t>("size");
    dao->hash_ = row.get<std::string>("hash");
    dao->note_ = row.get<std::string>("note");
    dao->dirty_ = false;
    dao->storedInDb_ = true;
    return dao;
}

}
}
