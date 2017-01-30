/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/codec/attachmentsplittingsink.h"

#include <algorithm>

#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/hex.h"

namespace Kullo {
namespace Codec {

AttachmentSplittingSink::AttachmentSplittingSink(
        const Db::SharedSessionPtr &session,
        id_type messageId,
        Codec::AttachmentProcessingStreamFactory &streamFactory)
    : session_(session)
    , streamFactory_(streamFactory)
{
    auto result = Dao::AttachmentDao::allForMessage(
                Dao::IsDraft::No, messageId, session_);
    while (auto dao = result->next())
    {
        attachmentDaos_.push_back(std::move(dao));
    }
    makeStream();
}

void AttachmentSplittingSink::write(
        const unsigned char *buffer, std::size_t size)
{
    if (!stream_) makeStream();

    auto bytesLeft = size;
    while (stream_ && bytesLeft > 0)
    {
        auto bytesToWrite = std::min(bytesLeft, bytesLeftInCurrentStream_);

        stream_->stream().write(buffer, bytesToWrite);
        buffer += bytesToWrite;
        bytesLeft -= bytesToWrite;
        bytesLeftInCurrentStream_ -= bytesToWrite;

        if (bytesLeftInCurrentStream_ == 0)
        {
            closeStream();
            makeStream();
        }
    }

    if (bytesLeft > 0)
    {
        throw Db::DatabaseIntegrityError(
                    "AttachmentSplittingSink: stream is too long");
    }
}

void AttachmentSplittingSink::close()
{
    // drain trailing empty attachments from queue
    while (stream_ && bytesLeftInCurrentStream_ == 0)
    {
        closeStream();
        makeStream();
    }
    closeStream();

    if (!attachmentDaos_.empty() || bytesLeftInCurrentStream_ > 0)
    {
        throw Db::DatabaseIntegrityError(
                    "AttachmentSplittingSink: stream is too short");
    }
}

void AttachmentSplittingSink::makeStream()
{
    if (attachmentDaos_.empty()) return;

    auto &dao = attachmentDaos_.front();
    bytesLeftInCurrentStream_ = dao->size();
    stream_ = streamFactory_.makeStream(
                session_,
                dao->index(),
                bytesLeftInCurrentStream_,
                Util::Hex::decode(dao->hash()));

    attachmentDaos_.pop_front();
}

void AttachmentSplittingSink::closeStream()
{
    if (stream_)
    {
        stream_->stream().close();
        stream_.reset();
    }
}

}
}
