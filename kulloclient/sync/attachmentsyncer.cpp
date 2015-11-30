/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/sync/attachmentsyncer.h"

#include <smartsqlite/exceptions.h>

#include "kulloclient/codec/exceptions.h"
#include "kulloclient/codec/messagecompressor.h"
#include "kulloclient/crypto/hasher.h"
#include "kulloclient/crypto/symmetriccryptor.h"
#include "kulloclient/crypto/symmetrickeyloader.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/dao/messagedao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/protocol/messagesclient.h"
#include "kulloclient/sync/exceptions.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/events.h"
#include "kulloclient/util/gzip.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/limits.h"
#include "kulloclient/util/usersettings.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

AttachmentSyncer::AttachmentSyncer(
        const Util::UserSettings &settings,
        Db::SharedSessionPtr session)
    : session_(session)
{
    client_.reset(new Protocol::MessagesClient(
                     *settings.address,
                     *settings.masterKey));
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

    // only download attachments if they are non-empty
    if (totalSize > 0)
    {
        // download attachments for the given message
        auto attachments = client_->getMessageAttachments(msgId);

        // decrypt/decompress and store attachments
        auto content = decodeAttachments(*msg, attachments.attachments);
        storeAttachments(*msg, std::move(attachmentDaos), content);
    }
    else
    {
        storeAttachments(*msg, std::move(attachmentDaos), {});
    }
}

std::vector<unsigned char> AttachmentSyncer::decodeAttachments(
        const Dao::MessageDao &message,
        const std::vector<unsigned char> &attachments)
{
    if (attachments.size() == 0) return {{}};

    if (static_cast<size_t>(attachments.size())
            > MESSAGE_ATTACHMENTS_MAX_BYTES)
    {
        throw Codec::InvalidContentFormat(
                    std::string("attachments too large: ") +
                    std::to_string(attachments.size()));
    }

    Crypto::SymmetricCryptor cryptor;
    auto content = cryptor.decrypt(
                attachments,
                Crypto::SymmetricKeyLoader::fromVector(message.symmetricKey()),
                Util::to_vector(Crypto::SymmetricCryptor::ATTACHMENTS_IV)
                );
    return Codec::MessageCompressor().decompress(content);
}

void AttachmentSyncer::storeAttachments(
        const Dao::MessageDao &message,
        std::vector<std::unique_ptr<Dao::AttachmentDao>> attachments,
        const std::vector<unsigned char> &content)
{
    auto attBegin = content.begin();
    auto attEnd = content.begin();
    for (auto &att : attachments)
    {
        attEnd = attBegin + att->size();
        if (attEnd > content.end())
        {
            throw Codec::InvalidContentFormat("attachment data is too short");
        }
        std::vector<unsigned char> attContent(attBegin, attEnd);
        attBegin = attEnd;
        if (att->hash() != Crypto::Hasher::sha512Hex(attContent))
        {
            throw Codec::InvalidContentFormat("attachment digest doesn't match");
            return;
        }
        att->setContent(attContent);
    }
    if (attEnd != content.end())
    {
        throw Codec::InvalidContentFormat("attachment data is too long");
    }

    EMIT(events.messageAttachmentsDownloaded,
         message.conversationId(),
         message.id());
}

}
}
