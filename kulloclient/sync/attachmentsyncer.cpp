/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/sync/attachmentsyncer.h"

#include <smartsqlite/exceptions.h>

#include "kulloclient/codec/attachmentprocessingstreamfactory.h"
#include "kulloclient/codec/attachmentsplittingsink.h"
#include "kulloclient/codec/sizelimitingfilter.h"
#include "kulloclient/crypto/decryptingfilter.h"
#include "kulloclient/crypto/exceptions.h"
#include "kulloclient/crypto/hashverifyingfilter.h"
#include "kulloclient/crypto/symmetrickeyloader.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/dao/messageattachmentsink.h"
#include "kulloclient/dao/messagedao.h"
#include "kulloclient/http/TransferProgress.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/protocol/messagesclient.h"
#include "kulloclient/sync/definitions.h"
#include "kulloclient/sync/exceptions.h"
#include "kulloclient/util/events.h"
#include "kulloclient/util/filterchain.h"
#include "kulloclient/util/gzipdecompressingfilter.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/limits.h"
#include "kulloclient/util/usersettings.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

AttachmentSyncer::AttachmentSyncer(
        const Util::Credentials &credentials,
        const Db::SharedSessionPtr &session,
        const std::shared_ptr<Http::HttpClient> &httpClient)
    : session_(session)
{
    messagesClient_.reset(new Protocol::MessagesClient(
                     *credentials.address,
                     *credentials.masterKey,
                      httpClient));
}

AttachmentSyncer::~AttachmentSyncer()
{
}

SyncIncomingAttachmentsProgress AttachmentSyncer::initialProgress()
{
    SyncIncomingAttachmentsProgress progress;
    progress.downloadedBytes = 0;
    progress.totalBytes = Dao::AttachmentDao::sizeOfAllDownloadable(session_);
    return progress;
}

void AttachmentSyncer::run(std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    while (true)
    {
        if (*shouldCancel) throw SyncCanceled();

        estimatedRemaining_ = Dao::AttachmentDao::sizeOfAllDownloadable(session_);

        progress_.downloadedBytes = previouslyDownloaded_;
        progress_.totalBytes = previouslyDownloaded_ + estimatedRemaining_;
        EMIT(events.progressed, progress_);

        auto msgId = Dao::AttachmentDao::msgIdForFirstDownloadable(session_);
        if (msgId <= 0) return;

        if (*shouldCancel) throw SyncCanceled();

        previouslyDownloaded_ += downloadForMessage(msgId, shouldCancel);
    }
}

size_t AttachmentSyncer::downloadForMessage(
        int64_t msgId, std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    auto msg = Dao::MessageDao::load(msgId, Dao::Old::No, session_);
    if (!msg)
    {
        Log.d() << "Tried to download attachments for nonexisting message "
                << msgId;
        return 0;
    }

    if (Dao::AttachmentDao::allAttachmentsDownloaded(msgId, session_))
    {
        Log.d() << "Tried to download already downloaded attachments "
                << "for message " << msgId;
        return 0;
    }

    if (*shouldCancel) throw SyncCanceled();

    // get all attachments from DB
    auto attachmentDaoResult = Dao::AttachmentDao::allForMessage(
                Dao::IsDraft::No,
                msg->id(),
                session_);
    std::vector<std::unique_ptr<Dao::AttachmentDao>> attachmentDaos;
    size_t totalSize = 0;
    while (auto dao = attachmentDaoResult->next())
    {
        totalSize += dao->size();
        attachmentDaos.push_back(std::move(dao));
    }

    // for downloading attachments of a single message: initialize estimate
    if (estimatedRemaining_ == 0) {
        estimatedRemaining_ = totalSize;
    }

    if (*shouldCancel) throw SyncCanceled();

    // create processing stream
    auto key = Crypto::SymmetricKeyLoader::fromVector(msg->symmetricKey());
    Codec::AttachmentProcessingStreamFactory streamFactory(msgId);
    Util::FilterChain stream(
                make_unique<Codec::AttachmentSplittingSink>(
                    session_, msgId, streamFactory));
    stream.pushFilter(make_unique<Util::GZipDecompressingFilter>());
    stream.pushFilter(make_unique<Crypto::DecryptingFilter>(key));
    stream.pushFilter(make_unique<Codec::SizeLimitingFilter>(
                       MESSAGE_ATTACHMENTS_MAX_BYTES));

    size_t downloadedBytes = 0;
    try
    {
        // only download attachments if they are non-empty
        if (totalSize > 0)
        {
            if (*shouldCancel) throw SyncCanceled();

            // download attachments for the given message
            auto attachments = messagesClient_->getMessageAttachments(
                        msgId,
                        [this, totalSize, shouldCancel]
                        (const Http::TransferProgress &progress)
            {
                // make sure downloadTotal has a sane value
                auto downloadTotal = std::max(
                            progress.downloadTotal,
                            progress.downloadTransferred);

                int64_t improvedEstimate;
                if (downloadTotal > 0)
                {
                    // Remove the current download's impact on the estimate,
                    // replace by value returned by the HTTP client.
                    improvedEstimate = estimatedRemaining_
                            - totalSize + downloadTotal;
                }
                else
                {
                    improvedEstimate = estimatedRemaining_;
                }

                SyncIncomingAttachmentsProgress newProgress;
                newProgress.downloadedBytes =
                        previouslyDownloaded_ + progress.downloadTransferred;
                newProgress.totalBytes = previouslyDownloaded_ + improvedEstimate;

                if (newProgress != progress_)
                {
                    progress_ = newProgress;
                    EMIT(events.progressed, progress_);
                }

                if (*shouldCancel) messagesClient_->cancel();
            });

            // decrypt/decompress and store attachments
            downloadedBytes = attachments.attachments.size();
            stream.write(attachments.attachments);
        }
        stream.close();
    }
    catch (Protocol::Canceled&)
    {
        throw SyncCanceled();
    }
    catch (Crypto::IntegrityFailure&)
    {
        Log.e() << "Integrity failure while decrypting attachments for "
                << "message " << std::to_string(msgId) << ", skipping.";
        return downloadedBytes;
    }
    catch (Util::GZipStreamError &ex)
    {
        Log.e() << "Error while decompressing attachments for "
                << "message " << std::to_string(msgId) << ", skipping.\n"
                << "Exception: " << formatException(ex);
        return downloadedBytes;
    }

    EMIT(events.messageAttachmentsDownloaded,
         msg->conversationId(),
         msg->id());
    return downloadedBytes;
}

}
}
