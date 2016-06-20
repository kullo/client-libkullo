/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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
#include "kulloclient/protocol/messagesclient.h"
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
    client_.reset(new Protocol::MessagesClient(
                     *credentials.address,
                     *credentials.masterKey,
                      httpClient));
}

AttachmentSyncer::~AttachmentSyncer()
{
}

void AttachmentSyncer::run(std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    while (true)
    {
        if (*shouldCancel) throw SyncCanceled();

        auto msgId = Dao::AttachmentDao::msgIdForFirstDownloadable(session_);
        if (msgId <= 0) return;

        downloadForMessage(msgId, shouldCancel);
    }
}

void AttachmentSyncer::downloadForMessage(
        int64_t msgId, std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    auto msg = Dao::MessageDao::load(msgId, Dao::Old::No, session_);
    if (!msg)
    {
        Log.d() << "Tried to download attachments for nonexisting message "
                << msgId;
        return;
    }

    if (Dao::AttachmentDao::allAttachmentsDownloaded(msgId, session_))
    {
        Log.d() << "Tried to download already downloaded attachments "
                << "for message " << msgId;
        return;
    }

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

    try
    {
        // only download attachments if they are non-empty
        if (totalSize > 0)
        {
            // download attachments for the given message
            auto attachments = client_->getMessageAttachments(msgId);

            // decrypt/decompress and store attachments
            stream.write(attachments.attachments);
        }
        stream.close();
    }
    catch (Crypto::IntegrityFailure&)
    {
        Log.e() << "Integrity failure while decrypting attachments for "
                << "message " << std::to_string(msgId) << ", skipping.";
        return;
    }

    EMIT(events.messageAttachmentsDownloaded,
         msg->conversationId(),
         msg->id());
}

}
}
