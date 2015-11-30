/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/draftattachmentsaddworker.h"

#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/crypto/hasher.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/event/draftattachmentaddedevent.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/filesystem.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/scopedbenchmark.h"

namespace fs = boost::filesystem;

namespace Kullo {
namespace ApiImpl {

DraftAttachmentsAddWorker::DraftAttachmentsAddWorker(
        int64_t convId,
        const std::string &path,
        const std::string &mimeType,
        const std::string &dbPath,
        std::shared_ptr<Api::SessionListener> sessionListener,
        std::shared_ptr<Api::DraftAttachmentsAddListener> listener)
    : convId_(convId)
    , path_(path)
    , mimeType_(mimeType)
    , dbPath_(dbPath)
    , sessionListener_(sessionListener)
    , listener_(listener)
{}

void DraftAttachmentsAddWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("AttAddW");

    try
    {
        auto path = fs::path(path_);
        if (!(fs::exists(path) && fs::is_regular_file(path)))
        {
            throw Util::FilesystemError(
                        std::string("File doesn't exist: ") + path_);
        }

        uint64_t attachmentId;

        // copy attachment from filesystem to DB
        {
            Util::ScopedBenchmark benchmark("Copy attachment from filesystem to DB");
            K_RAII(benchmark);

            std::ifstream istream(path_, std::ios::binary);
            if (istream.fail())
            {
                throw Util::FilesystemError(
                            std::string("File could not be opened: ") + path_);
            }

            auto hash = Crypto::Hasher::sha512Hex(istream);

            auto session = Db::makeSession(dbPath_);
            SmartSqlite::ScopedTransaction tx(session, SmartSqlite::Immediate);

            attachmentId = Dao::AttachmentDao::idForNewDraftAttachment(
                        convId_, session);

            Dao::AttachmentDao dao(session);
            dao.setDraft(true);
            dao.setMessageId(convId_);
            dao.setIndex(attachmentId);
            dao.setFilename(Util::Filesystem::filename(path_));
            dao.setMimeType(mimeType_);
            dao.setSize(fs::file_size(path));
            dao.setHash(hash);
            dao.save();

            istream.clear();
            istream.seekg(0);
            dao.setContent(istream);

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
