/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/draftattachmentsaddworker.h"

#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/crypto/hasher.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/event/draftattachmentaddedevent.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/filesystem.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/scopedbenchmark.h"

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

        uint64_t attachmentId;

        // copy attachment from filesystem to DB
        {
            Util::ScopedBenchmark benchmark("Copy attachment from filesystem to DB");
            K_RAII(benchmark);

            auto istream = Util::Filesystem::makeIfstream(path_);
            auto hash = Crypto::Hasher::sha512Hex(*istream);

            auto session = Db::makeSession(sessionConfig_);
            SmartSqlite::ScopedTransaction tx(session, SmartSqlite::Immediate);

            attachmentId = Dao::AttachmentDao::idForNewDraftAttachment(
                        convId_, session);

            Dao::AttachmentDao dao(session);
            dao.setDraft(true);
            dao.setMessageId(convId_);
            dao.setIndex(attachmentId);
            dao.setFilename(Util::Filesystem::filename(path_));
            dao.setMimeType(mimeType_);
            dao.setSize(Util::Filesystem::fileSize(path_));
            dao.setHash(hash);
            dao.save();

            istream->clear();
            istream->seekg(0);
            dao.setContent(*istream);

            tx.commit();
        }

        // send updated state to model
        {
            std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
            if (listener_)
            {
                listener_->finished(convId_, attachmentId, path_);
            }
            if (sessionListener_)
            {
                sessionListener_->internalEvent(
                            std::make_shared<Event::DraftAttachmentAddedEvent>(
                                Data::DraftAttachment(convId_, attachmentId)));
            }
        }
    }
    catch(std::exception &ex)
    {
        Log.e() << "DraftAttachmentsAddWorker failed: "
                << Util::formatException(ex);

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->error(
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
