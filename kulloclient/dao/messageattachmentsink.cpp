/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/messageattachmentsink.h"

#include <smartsqlite/connection.h>
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Dao {


MessageAttachmentSink::MessageAttachmentSink(
        const Db::SharedSessionPtr &session,
        id_type messageId,
        id_type attachmentIndex,
        size_t attachmentSize)
    : session_(session)
    , messageId_(messageId)
    , attachmentIndex_(attachmentIndex)
    , attachmentSize_(attachmentSize)
{}

void MessageAttachmentSink::write(
        const unsigned char *buffer, std::size_t writeSize)
{
    openIfNotOpen();

    if (handle_->bytesWritten + writeSize > attachmentSize_)
    {
        throw Db::DatabaseIntegrityError(
                    "MessageAttachmentSink::write: The stored size is smaller "
                    "than the stream length");
    }
    handle_->blob.write(buffer, writeSize, handle_->bytesWritten);
    handle_->bytesWritten += writeSize;
}

void MessageAttachmentSink::close()
{
    openIfNotOpen();

    auto bytesWritten = handle_->bytesWritten;
    handle_.reset();

    if (bytesWritten < attachmentSize_)
    {
        throw Db::DatabaseIntegrityError(
                    "MessageAttachmentSink::close: The stored size is larger "
                    "than the stream length");
    }
}

void MessageAttachmentSink::openIfNotOpen()
{
    if (handle_) return;

    // get rowid
    int64_t rowid;
    {
        auto stmt = session_->prepare(
                    "SELECT rowid FROM attachments_content "
                    "WHERE draft = :draft "
                        "AND message_id = :message_id "
                        "AND idx = :idx");
        stmt.bind(":draft", 0);
        stmt.bind(":message_id", messageId_);
        stmt.bind(":idx", attachmentIndex_);
        rowid = stmt.execWithSingleResult().get<int64_t>(0);
    }

    // set size of attachment
    {
        auto stmt = session_->prepare(
                    "UPDATE attachments_content "
                    "SET content = zeroblob(:size) "
                    "WHERE rowid = :rowid");
        stmt.bind(":size", attachmentSize_);
        stmt.bind(":rowid", rowid);
        stmt.execWithoutResult();
    }

    // open SQLite BLOB
    auto blob = session_->openBlob(
                "main", "attachments_content", "content", rowid,
                SmartSqlite::Blob::READWRITE);

    handle_ = make_unique<Handle>(std::move(blob));
}

}
}
