/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/draftattachmentsaddworker.h"

#include <boost/optional.hpp>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/crypto/hasher.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/event/draftattachmentaddedevent.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/filesystem.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/limits.h"
#include "kulloclient/util/multithreading.h"
#include "kulloclient/util/scopedbenchmark.h"
#include "kulloclient/util/strings.h"

namespace Kullo {
namespace ApiImpl {

DraftAttachmentsAddWorker::DraftAttachmentsAddWorker(
        int64_t convId,
        const std::string &path,
        const std::string &mimeType,
        const Db::SessionConfig &sessionConfig,
        std::shared_ptr<Api::SessionListener> sessionListener,
        std::shared_ptr<Api::DraftAttachmentsAddListener> listener)
    : convId_(convId)
    , path_(path)
    , mimeType_(mimeType)
    , sessionConfig_(sessionConfig)
    , sessionListener_(sessionListener)
    , listener_(listener)
{}

void DraftAttachmentsAddWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("AttAddW");

    try
    {
        if (!(Util::Filesystem::exists(path_) && Util::Filesystem::isRegularFile(path_)))
        {
            throw Util::FilesystemError(
                        std::string("File doesn't exist: ") + path_);
        }

        auto session = Db::makeSession(sessionConfig_);
        auto sizeOfExisting = Dao::AttachmentDao::sizeOfAllForMessage(
                    Dao::IsDraft::Yes, convId_, session);
        auto newAttachmentSize = Util::Filesystem::fileSize(path_);
        auto totalSize = sizeOfExisting + newAttachmentSize;
        if (totalSize > Util::MESSAGE_ATTACHMENTS_MAX_BYTES)
        {
            throw Util::FileTooBigError(
                        std::string("Existing attachments: ") +
                        Util::Strings::formatReadable(sizeOfExisting) + ", " +
                        "new attachment: " +
                        Util::Strings::formatReadable(newAttachmentSize) + ", " +
                        "sum: " +
                        Util::Strings::formatReadable(totalSize) + ", " +
                        "allowed: " +
                        Util::Strings::formatReadable(Util::MESSAGE_ATTACHMENTS_MAX_BYTES));
        }

        uint64_t attachmentId;

        // copy attachment from filesystem to DB
        {
            Util::ScopedBenchmark benchmark("Copy attachment from filesystem to DB");
            K_RAII(benchmark);

            auto istream = Util::Filesystem::makeIfstream(path_);

            SmartSqlite::ScopedTransaction tx(session, SmartSqlite::Immediate);

            attachmentId = Dao::AttachmentDao::idForNewDraftAttachment(
                        convId_, session);

            Dao::AttachmentDao dao(session);
            dao.setDraft(true);
            dao.setMessageId(convId_);
            dao.setIndex(attachmentId);
            dao.setFilename(Util::Filesystem::filename(path_));
            dao.setMimeType(mimeType_);
            dao.setSize(newAttachmentSize);
            dao.save();

            Crypto::Sha512Hasher hasher;
            boost::optional<Dao::Progress> lastListenerCall;
            const auto LISTENER_CALL_INTERVAL = 1*1024*1024; /* 1 MiB */
            dao.setContent(*istream, [&](const unsigned char *data, std::size_t length, const Dao::Progress &progress) -> void {
                hasher.update(data, length);

                if (!lastListenerCall ||
                        lastListenerCall->bytesProcessed+LISTENER_CALL_INTERVAL < progress.bytesProcessed)
                {
                    if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
                    {
                        listener->progressed(convId_,
                                             attachmentId,
                                             progress.bytesProcessed,
                                             progress.bytesTotal);
                    }
                    lastListenerCall = progress;
                }
            });
            dao.setHash(hasher.hexDigest());
            dao.save();

            tx.commit();
        }

        // send updated state to model
        {
            if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
            {
                listener->finished(convId_, attachmentId, path_);
            }
            if (auto sessionListener = Util::copyGuardedByMutex(sessionListener_, mutex_))
            {
                sessionListener->internalEvent(
                            nn_make_shared<Event::DraftAttachmentAddedEvent>(
                                Data::DraftAttachment(convId_, attachmentId)));
            }
        }
    }
    catch(std::exception &ex)
    {
        Log.e() << "DraftAttachmentsAddWorker failed: "
                << Util::formatException(ex);

        if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
        {
            listener->error(
                        convId_, path_,
                        toLocalError(std::current_exception()));
        }
    }
}

void DraftAttachmentsAddWorker::cancel()
{
    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    sessionListener_.reset();
    listener_.reset();
}

}
}
