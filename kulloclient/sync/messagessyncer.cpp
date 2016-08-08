/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/sync/messagessyncer.h"

#include <thread>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/codec/exceptions.h"
#include "kulloclient/codec/messagedecoder.h"
#include "kulloclient/codec/messagedecryptor.h"
#include "kulloclient/codec/messageencoder.h"
#include "kulloclient/codec/messageencryptor.h"
#include "kulloclient/codec/privatekeyprovider.h"
#include "kulloclient/dao/deliverydao.h"
#include "kulloclient/dao/participantdao.h"
#include "kulloclient/dao/publickeydao.h"
#include "kulloclient/dao/symmetrickeydao.h"
#include "kulloclient/dao/syncdao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/protocol/messagesclient.h"
#include "kulloclient/util/events.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/sync/exceptions.h"

using namespace Kullo::Dao;
using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

namespace {
const std::chrono::milliseconds SQLITE_MAX_BUSY_INTERVAL(100);
}

MessagesSyncer::MessagesSyncer(
        const Credentials &credentials,
        const std::shared_ptr<Codec::PrivateKeyProvider> &privKeyProvider,
        const Db::SharedSessionPtr &session,
        const std::shared_ptr<Http::HttpClient> &httpClient)
    : pubKeysClient_(httpClient)
    , session_(session)
    , credentials_(credentials)
    , privKeyProvider_(privKeyProvider)
{
    kulloAssert(session_);

    client_.reset(new Protocol::MessagesClient(
                     *credentials_.address,
                     *credentials_.masterKey,
                      httpClient));

    msgAdder_.events.conversationAdded = forwardEvent(events.conversationAdded);
    msgAdder_.events.conversationModified = forwardEvent(events.conversationModified);
    msgAdder_.events.messageAdded = forwardEvent(events.messageAdded);
    msgAdder_.events.senderAdded = forwardEvent(events.senderAdded);
}

MessagesSyncer::~MessagesSyncer()
{
}

void MessagesSyncer::run(std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    shouldCancel_ = shouldCancel;

    Log.d() << "Sync: Starting downloadAndProcessMessages() ...";
    downloadAndProcessMessages();

    Log.d() << "Sync: Starting uploadModifications() ...";
    uploadModifications();

    Log.d() << "Sync: Done syncing messages.";

    EMIT(events.finished, progress_);

    lastYield_ = clock::now();
}

void MessagesSyncer::downloadAndProcessMessages()
{
    /*
     *                      | no local copy           | local copy                 |
     * ---------------------|-------------------------|----------------------------|
     * Remotely deleted     | 1. Download tombstone   | 3. Clear local message     |
     *                      |    handleNewTombstone() |    handleDeletedMessage()  |
     * ---------------------|-------------------------|----------------------------|
     * Remotely not deleted | 2. Download message     | 4. Merge                   |
     *                      |    handleNewMessage()   |    handleModifiedMessage() |
     *
     * For 1. and 3., no MessageDecoder is needed.
     *
     * 1. emits: ––
     * 2. emits: messageAdded, (conversationModified or conversationAdded)
     * 3. emits: messageDeleted
     * 4. emits: messageModified (iff content changed), conversationModified (is state changed)
     *
     */
    Protocol::MessagesClient::GetMessagesResult result;
    do
    {
        result = client_->getMessages(SyncDao::timestamp(SyncDao::Message, session_));


        /*
         *              lllllllllllll
         * ppppppppppppppppppp
         *              xxxxxx
         * tttttttttttttttttttttttttt
         *
         * L: countLeft
         * P: countProcessed
         * X: countProcessedFromLeft
         * T: L + P - X
         *
         * ratio = P / T
         *
         */
        progress_.countLeft = result.countLeft;
        progress_.countTotal = progress_.countLeft + progress_.countProcessed;
        EMIT(events.progressed, progress_); // I know how much I have to do

        for (const Protocol::Message &httpMsg : result.messages)
        {
            if (*shouldCancel_) throw SyncCanceled();

            auto local = MessageDao::load(httpMsg.id, Old::No, session_);
            if      (!local &&  httpMsg.deleted) handleNewTombstone(httpMsg);            // 1. Download tombstone
            else if (!local && !httpMsg.deleted) handleNewMessage(httpMsg);              // 2. Download message
            else if ( local &&  httpMsg.deleted) handleDeletedMessage(httpMsg, *local);  // 3. Clear local message
            else if ( local && !httpMsg.deleted) handleModifiedMessage(httpMsg, *local); // 4. Merge
            else kulloAssertionFailed("Logic error");

            ++progress_.countProcessed;
            EMIT(events.progressed, progress_);

            Dao::SyncDao::setTimestamp(Dao::SyncDao::Message, httpMsg.lastModified, session_);
            yieldIfNecessary();
        }
    }
    while (result.countReturned < result.countLeft);
}

void MessagesSyncer::handleNewTombstone(const Protocol::Message &httpMsg)
{
    SmartSqlite::ScopedTransaction tx(session_, SmartSqlite::Immediate);
    MessageDao tombstone(session_);
    tombstone.setId(httpMsg.id);
    tombstone.setLastModified(httpMsg.lastModified);
    tombstone.setDeleted(true);
    tombstone.save(CreateOld::No);
    tx.commit();
}

void MessagesSyncer::handleNewMessage(const Protocol::Message &httpMsg)
{
    auto decryptedMessage = tryDecrypt(httpMsg);
    if (!decryptedMessage) return;

    SmartSqlite::ScopedTransaction tx(session_, SmartSqlite::Immediate);
    auto decoder = tryDecode(*decryptedMessage, tx);
    if (!decoder) return;

    decoder->makeConversation();

    auto &remote = decoder->message();
    msgAdder_.addMessage(
                remote,
                decoder->conversation(),
                decoder->sender(),
                decoder->avatar(),
                decoder->attachments(),
                session_);

    // trigger upgrade of the meta format if it is smaller than supported
    if (remote.metaVersion() < Codec::LATEST_META_VERSION)
    {
        remote.setMetaVersion(Codec::LATEST_META_VERSION);
        remote.save(CreateOld::Yes);
    }

    tx.commit();

    ++progress_.countNew;
    if (!remote.state(MessageState::Read)) ++progress_.countNewUnread;

    msgAdder_.emitSignals();
}

void MessagesSyncer::handleDeletedMessage(const Protocol::Message &httpMsg, MessageDao &local)
{
    // A remotely deleted message overrides all local changes
    // because it cannot be restored on the server
    id_type messageId;
    id_type conversationId;

    SmartSqlite::ScopedTransaction tx(session_, SmartSqlite::Immediate);

    // delete dependent data
    AttachmentDao::deleteAttachmentsForMessage(local.id(), session_);
    auto sender = ParticipantDao::load(local.id(), session_);
    if (sender) sender->deletePermanently();

    messageId = local.id();
    conversationId = local.conversationId();

    local.dropOld();
    local.clearData();
    local.setLastModified(httpMsg.lastModified);
    local.setDeleted(true);
    local.save(CreateOld::No);

    tx.commit();

    ++progress_.countDeleted;
    EMIT(events.messageDeleted, conversationId, messageId);
}

void MessagesSyncer::handleModifiedMessage(const Protocol::Message &httpMsg, MessageDao &local)
{
    auto decryptedMessage = tryDecrypt(httpMsg);
    if (!decryptedMessage) return;

    SmartSqlite::ScopedTransaction tx(session_, SmartSqlite::Immediate);
    auto decoder = tryDecode(*decryptedMessage, tx);
    if (!decoder) return;

    decoder->makeConversation();

    bool stateModified = false;
    bool localModified = false;

    auto &remote = decoder->message();
    auto old = MessageDao::load(remote.id(), Old::Yes, session_);
    if (!old) // no local modifications, apply remote modifications
    {
        local.setLastModified(remote.lastModified());
        stateModified =
                local.setState(MessageState::Read, remote.state(MessageState::Read))
                || stateModified;
        stateModified =
                local.setState(MessageState::Done, remote.state(MessageState::Done))
                || stateModified;
    }
    else // merge conflicting modifications
    {
        local.setLastModified(remote.lastModified());

        // only merge if message has not been deleted locally (local deletion > remote modification)
        if (!local.deleted())
        {
            // sync states: local modification > remote modification
            for (int stateIter = 0; stateIter < static_cast<int>(MessageState::Any); ++stateIter)
            {
                // only use remote state if state was only modified on the server
                MessageState state = static_cast<MessageState>(stateIter);
                if ((local.state(state) != remote.state(state)) &&
                        (old->state(state) == local.state(state)))
                {
                    stateModified = local.setState(state, remote.state(state)) || stateModified;
                }
            }
        }

        if (local == *old)
        {
            local.dropOld();
        }
        else
        {
            // update old with remote
            old->setLastModified(remote.lastModified());
            old->setDeleted(remote.deleted());
            old->setState(MessageState::Read, remote.state(MessageState::Read));
            old->setState(MessageState::Done, remote.state(MessageState::Done));
            old->save(CreateOld::No);
        }
    }

    localModified = local.save(CreateOld::No);

    tx.commit();

    if (localModified)
    {
        ++progress_.countModified;
        EMIT(events.messageModified, local.conversationId(), local.id());
    }
    if (stateModified)
    {
        EMIT(events.conversationModified, local.conversationId());
    }
}

void MessagesSyncer::downloadPublicKey(
        const Util::KulloAddress &address, id_type sigKeyId)
{
    auto keyPair = pubKeysClient_.getPublicKey(address, sigKeyId);

    // Re-downloaded key must not be stored again to avoid
    // SQLITE_CONSTRAINT_PRIMARYKEY. Public key is re-downloaded when syncer is
    // cancelled while processing messages for a given key.
    if (!Dao::PublicKeyDao::load(
                address, Dao::PublicKeyDao::SIGNATURE, keyPair.id, session_))
    {
        Dao::PublicKeyDao keyDao(address, session_);
        keyDao.setKeyType(Dao::PublicKeyDao::SIGNATURE);
        keyDao.setId(keyPair.id);
        keyDao.setKey(keyPair.pubkey);
        keyDao.save();
    }
}

void MessagesSyncer::uploadModifications()
{
    for (auto id : MessageDao::locallyModifiedMessages(Codec::LATEST_META_VERSION, session_))
    {
        if (*shouldCancel_) throw SyncCanceled();

        auto local = MessageDao::load(id, Old::No, session_);
        if (!local)
        {
            throw Db::DatabaseIntegrityError(
                        "MessagesSyncer::processNextModification(): "
                        "Couldn't load enqueued message."
                        );
        }
        auto old = MessageDao::load(id, Old::Yes, session_);
        if (!old)
        {
            Log.e() << "`old` does not exist but message ID was in locallyModifiedMessages(). "
                    << "Did someone delete `old`?! Skip for now.";
            continue;
        }

        // Skip messages where there is no difference between `old` and `local`
        if (*local == *old) continue;

        if (local->deleted())
        {
            Protocol::IdLastModified idlm;
            idlm.id           = old->id();
            idlm.lastModified = old->lastModified();
            try
            {
                handleSuccessfulModification(client_->deleteMessage(idlm));
            }
            catch (Protocol::Conflict)
            {
                // new version will be downloaded on next sync
                continue;
            }
        }
        else
        {
            auto encodedMeta = Codec::MessageEncoder::encodeMeta(
                        local->state(MessageState::Read),
                        local->state(MessageState::Done),
                        DeliveryDao(session_).loadDelivery(local->id()));

            auto meta = Codec::MessageEncryptor().encryptMeta(
                        encodedMeta,
                        getPrivateDataKey());

            Protocol::IdLastModified idlm;
            idlm.id           = old->id();
            idlm.lastModified = old->lastModified();
            try
            {
                handleSuccessfulModification(client_->modifyMeta(idlm, meta));
            }
            catch (Protocol::Conflict)
            {
                // new version will be downloaded on next sync
                continue;
            }
        }
    }
}

void MessagesSyncer::handleSuccessfulModification(const Protocol::IdLastModified &idlm)
{
    const std::string errmsg = "MessagesSyncer::handleSuccessfulModification()";

    SmartSqlite::ScopedTransaction tx(session_, SmartSqlite::Immediate);
    auto msg = MessageDao::load(idlm.id, Old::No, session_);
    if (!msg)
    {
        throw Db::DatabaseIntegrityError(errmsg + " Couldn't load modified message.");
    }

    msg->setLastModified(idlm.lastModified);
    msg->save(CreateOld::No);
    msg->dropOld();
    tx.commit();

    if (!msg->deleted())
    {
        // lastModified is changed by the server
        // Notify client s.th. Message model can be reloaded and have
        // the correct lastModified in it's dao's memory.
        EMIT(events.messageModified, msg->conversationId(), msg->id());
    }
}

void MessagesSyncer::yieldIfNecessary()
{
    // hack to give foreground process enough time to mark message as read, hopefully prevents SQLITE_BUSY...
    if (clock::now() - lastYield_ > 2 * SQLITE_MAX_BUSY_INTERVAL)
    {
        std::this_thread::sleep_for(SQLITE_MAX_BUSY_INTERVAL);
        lastYield_ = clock::now();
    }
}

Crypto::SymmetricKey MessagesSyncer::getPrivateDataKey()
{
    if (!privateDataKey_)
    {
        privateDataKey_ = make_unique<Crypto::SymmetricKey>(
                    Dao::SymmetricKeyDao::loadKey(
                        Dao::SymmetricKeyDao::PRIVATE_DATA_KEY,
                        session_));
    }

    return *privateDataKey_;
}

std::unique_ptr<Codec::MessageDecoder> MessagesSyncer::makeDecoder(
        const Codec::DecryptedMessage &decryptedMessage)
{
    return make_unique<Codec::MessageDecoder>(
                decryptedMessage,
                *credentials_.address,
                session_);
}

boost::optional<Codec::DecryptedMessage> MessagesSyncer::tryDecrypt(
        const Protocol::Message &httpMsg)
{
    try
    {
        Codec::MessageDecryptor decryptor(
                    httpMsg, privKeyProvider_, getPrivateDataKey());
        return decryptor.decrypt();
    }
    catch (Codec::DecryptionKeyMissing)
    {
        Log.w() << "Decryption key missing for message " << httpMsg.id;
        return {};
    }
}

std::unique_ptr<Codec::MessageDecoder> MessagesSyncer::tryDecode(
        const Codec::DecryptedMessage &decryptedMessage,
        SmartSqlite::ScopedTransaction &tx)
{
    auto decoder = makeDecoder(decryptedMessage);
    try
    {
        decoder->decode();
    }
    catch (Codec::DecryptionKeyMissing)
    {
        Log.w() << "Decryption key missing for message " << decryptedMessage.id;
        return {};
    }
    catch (Codec::UnsupportedContentVersion &e)
    {
        // we don't know how to decode this message, skip it
        Log.w() << "Unsupported content version of message " << decryptedMessage.id << "\n"
                << formatException(e);
        return {};
    }
    catch (Codec::InvalidContentFormat &e)
    {
        // bad message, delete it (not yet implemented, skip for now)
        Log.w() << "Unsupported content format of message " << decryptedMessage.id << "\n"
                << formatException(e);
        return {};
    }
    catch (Codec::SignatureVerificationFailed)
    {
        // bad message, delete it (not yet implemented, skip for now)
        Log.w() << "Signature verification of message " << decryptedMessage.id << " failed";
        return {};
    }
    catch (Codec::SignatureVerificationKeyMissing)
    {
        tx.rollback();

        try
        {
            downloadPublicKey(
                        decoder->sender().address(),
                        decoder->signatureKeyId());
        }
        catch (Protocol::NotFound)
        {
            Log.w() << "Signature verification key not found for message "
                    << decryptedMessage.id;
            return {};
        }

        tx = SmartSqlite::ScopedTransaction(session_, SmartSqlite::Immediate);
        return tryDecode(decryptedMessage, tx);
    }

    return decoder;
}

}
}
