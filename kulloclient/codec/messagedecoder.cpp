/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/codec/messagedecoder.h"

#include <algorithm>
#include <iterator>
#include <jsoncpp/jsoncpp.h>

#include "kulloclient/codec/exceptions.h"
#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/crypto/signer.h"
#include "kulloclient/crypto/symmetrickeygenerator.h"
#include "kulloclient/crypto/symmetrickeyloader.h"
#include "kulloclient/dao/asymmetrickeypairdao.h"
#include "kulloclient/dao/publickeydao.h"
#include "kulloclient/dao/symmetrickeydao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/gzip.h"
#include "kulloclient/util/jsonhelper.h"
#include "kulloclient/util/limits.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Dao;
using namespace Kullo::Util;

namespace Kullo {
namespace Codec {

MessageDecoder::MessageDecoder(
        const DecryptedMessage &message,
        KulloAddress userAddress,
        Db::SharedSessionPtr session,
        MessageCompressor *compressor)
    : compressor_(compressor)
    , decryptedMessage_(message)
    , userAddress_(userAddress)
    , session_(session)
{
    kulloAssert(session_);

    if (!compressor_) compressor_.reset(new MessageCompressor());
}

void MessageDecoder::decode(VerifySignature verify)
{
    // decode() must be only called once
    kulloAssert(!contentDecoded_);

    message_ = make_unique<MessageDao>(session_);
    message_->setId(decryptedMessage_.id);
    message_->setLastModified(decryptedMessage_.lastModified);
    message_->setDateReceived(decryptedMessage_.dateReceived.toString());
    message_->setSymmetricKey(decryptedMessage_.keySafe.symmKey.toVector());

    try
    {
        // content
        contentDecoded_ = true;
        parseContent(decryptedMessage_.content);

        if (verify == VerifySignature::True)
        {
            verifyContent(decryptedMessage_.content);
        }

        // meta
        parseMeta(decryptedMessage_.metaVersion, decryptedMessage_.meta);
    }
    catch (ConversionException&)
    {
        std::throw_with_nested(InvalidContentFormat("MessageDecoder::decode()"));
    }
}

MessageDao &MessageDecoder::message()
{
    kulloAssert(contentDecoded_);
    return *message_;
}

std::vector<Delivery> MessageDecoder::delivery() const
{
    kulloAssert(contentDecoded_);
    return delivery_;
}

void MessageDecoder::makeConversation()
{
    kulloAssert(contentDecoded_); // decode() must be called before makeConversation()
    kulloAssert(!madeConversation_);  // makeConversation must be the only called once

    // conversation
    auto conv = ConversationDao::load(participantAddresses_, session_);
    if (!conv)
    {
        conv.reset(new ConversationDao(session_));
        conv->setParticipants(participantAddresses_);
    }
    conversation_ = std::move(conv);

    madeConversation_ = true;
}

ConversationDao &MessageDecoder::conversation()
{
    kulloAssert(madeConversation_); // makeConversation() must be called before this function
    return *conversation_;
}

const ConversationDao &MessageDecoder::conversation() const
{
    kulloAssert(madeConversation_); // makeConversation() must be called before this function
    return *conversation_;
}

std::vector<std::unique_ptr<AttachmentDao>> &MessageDecoder::attachments()
{
    kulloAssert(contentDecoded_); // decode() must be called before this function
    return attachments_;
}

ParticipantDao &MessageDecoder::sender()
{
    kulloAssert(contentDecoded_); // decode() must be called before this function
    return *sender_;
}

const ParticipantDao &MessageDecoder::sender() const
{
    kulloAssert(contentDecoded_); // decode() must be called before this function
    return *sender_;
}

id_type MessageDecoder::signatureKeyId() const
{
    return decryptedMessage_.content.sigKeyId;
}

void MessageDecoder::parseContent(const DecryptedContent &content)
{
    auto data = compressor_->decompress(content.data);
    auto dataString = Util::to_string(data);
    Json::Value messageJson;
    FWD_NESTED(messageJson = Util::JsonHelper::readObject(dataString),
               Util::JsonException,
               InvalidContentFormat("content invalid"));

    // sender
    if (!messageJson["sender"].isObject())
    {
        throw InvalidContentFormat("sender is not an object");
    }
    parseSender(messageJson["sender"], decryptedMessage_.id);

    // participantAddresses
    participantAddresses_.clear();
    if (sender_->address() != userAddress_)
    {
        participantAddresses_.insert(sender_->address());
    }

    if (!messageJson["recipients"].isArray())
    {
        throw InvalidContentFormat("recipients is not an array");
    }
    if (messageJson["recipients"].empty())
    {
        throw InvalidContentFormat("recipients are empty");
    }
    for (const auto &val : messageJson["recipients"])
    {
        boost::optional<KulloAddress> address;
        FWD_NESTED(address = CheckedConverter::toKulloAddress(val),
                   ConversionException,
                   InvalidContentFormat("recipient address invalid"));

        if (*address != userAddress_)
        {
            participantAddresses_.insert(*address);
        }
    }

    // attachments
    parseAttachments(messageJson["attachmentsIndex"], decryptedMessage_.id);

    // message
    message_->setSender(sender_->address().toString());
    FWD_NESTED(message_->setDateSent(CheckedConverter::toDateTime(messageJson["dateSent"]).toString()),
            ConversionException, InvalidContentFormat("date sent invalid"));
    FWD_NESTED(message_->setText(CheckedConverter::toString(messageJson["text"], "")),
            ConversionException, InvalidContentFormat("text invalid"));
    FWD_NESTED(message_->setFooter(CheckedConverter::toString(messageJson["footer"], "")),
            ConversionException, InvalidContentFormat("footer invalid"));
}

void MessageDecoder::verifyContent(const DecryptedContent &content)
{
    auto sigKeyDao = Dao::PublicKeyDao::load(
                sender_->address(),
                Dao::PublicKeyDao::SIGNATURE,
                decryptedMessage_.content.sigKeyId,
                session_);
    if (!sigKeyDao)
    {
        throw SignatureVerificationKeyMissing();
    }
    Crypto::AsymmetricKeyLoader loader;
    auto sigKey = loader.loadPublicKey(
                Crypto::AsymmetricKeyType::Signature,
                sigKeyDao->key());

    Crypto::Signer signer;
    if (!signer.verify(content.data, content.signature, sigKey))
    {
        throw SignatureVerificationFailed();
    }
}

void MessageDecoder::parseAttachments(const Json::Value &attachmentsIndex, id_type messageId)
{
    for (const auto &attJson : attachmentsIndex)
    {
        auto index = attachments_.size();
        auto att = AttachmentDao::load(IsDraft::No, messageId, index, session_);
        if (!att)
        {
            att.reset(new AttachmentDao(session_));
            att->setDraft(false);
            att->setMessageId(messageId);
            att->setIndex(index);
        }

        FWD_NESTED(att->setFilename(CheckedConverter::toString(attJson["filename"])),
                ConversionException,
                InvalidContentFormat("attachment filename invalid"));
        FWD_NESTED(att->setMimeType(CheckedConverter::toString(attJson["mimeType"])),
                ConversionException,
                InvalidContentFormat("attachment mimeType invalid"));
        FWD_NESTED(att->setSize(CheckedConverter::toUint32(attJson["size"])),
                ConversionException,
                InvalidContentFormat("attachment size invalid"));
        FWD_NESTED(att->setNote(CheckedConverter::toString(attJson["note"], "")),
                ConversionException,
                InvalidContentFormat("attachment note invalid"));
        FWD_NESTED(att->setHash(CheckedConverter::toHexString(attJson["hash"])),
                ConversionException,
                InvalidContentFormat("attachment hash invalid"));
        attachments_.push_back(std::move(att));
    }
}

void MessageDecoder::parseSender(const Json::Value &sender, id_type messageId)
{
    boost::optional<KulloAddress> address;
    FWD_NESTED(address = CheckedConverter::toKulloAddress(sender["address"]),
            ConversionException,
            InvalidContentFormat("sender address invalid"));

    std::shared_ptr<ParticipantDao> part(ParticipantDao::load(messageId, session_));
    if (!part)
    {
        part.reset(new ParticipantDao(*address, session_));
        part->setMessageId(messageId);
    }
    sender_ = part;

    FWD_NESTED(sender_->setName(CheckedConverter::toString(sender["name"])),
            ConversionException,
            InvalidContentFormat("sender name invalid"));
    FWD_NESTED(sender_->setOrganization(CheckedConverter::toString(sender["organization"], "")),
            ConversionException,
            InvalidContentFormat("sender organization invalid"));

    auto avatarJson = sender["avatar"];
    if (!(avatarJson.isNull() || avatarJson.isObject()))
    {
        throw InvalidContentFormat("sender.avatar must be null or an object");
    }

    if (!avatarJson.isNull() && avatarJson.size() != 0)
    {
        // avatarJson is non-empty JSON object

        if (avatarJson.size() == 2)
        {
            FWD_NESTED(sender_->setAvatarMimeType(CheckedConverter::toString(avatarJson["mimeType"])),
                    ConversionException,
                    InvalidContentFormat("avatar mimeType invalid"));
            FWD_NESTED(sender_->setAvatar(CheckedConverter::toVector(avatarJson["data"])),
                    ConversionException,
                    InvalidContentFormat("avatar data invalid"));
        }
        else
        {
            throw InvalidContentFormat("sender.avatar must be empty or consist of mimeType and data");
        }
    }
}

void MessageDecoder::parseMeta(metaVersion_type metaVersion, const std::string &meta)
{
    if (userAddress_ == Util::KulloAddress(message_->sender()))
    {
        message_->setMetaVersion(LATEST_META_VERSION);
        message_->setState(Dao::MessageState::Read, true);
        message_->setState(Dao::MessageState::Done, true);
        return;
    }

    Json::Value metaJson;
    bool read;
    bool done;
    switch (metaVersion)
    {
    case 0:
        // first version, with static IV and read/done stored as true/false
        if (!meta.empty())
        {
            FWD_NESTED(metaJson = Util::JsonHelper::readObject(meta),
                       Util::JsonException,
                       InvalidContentFormat("meta (version 0) invalid"));
        }

        FWD_NESTED(read = CheckedConverter::toBool(metaJson["read"], false),
                ConversionException,
                InvalidContentFormat("meta (version 0) read invalid"));
        FWD_NESTED(done = CheckedConverter::toBool(metaJson["done"], false),
                ConversionException,
                InvalidContentFormat("meta (version 0) done invalid"));

        message_->setMetaVersion(metaVersion);
        message_->setState(Dao::MessageState::Read, read);
        message_->setState(Dao::MessageState::Done, done);

        break;

    case 1:
        // version with random IV and read/done stored as 0/1
        if (!meta.empty())
        {
            FWD_NESTED(metaJson = Util::JsonHelper::readObject(meta),
                       Util::JsonException,
                       InvalidContentFormat("meta (version 1) invalid"));
        }

        FWD_NESTED(read = (CheckedConverter::toUint32(metaJson["read"], 0) != 0),
                ConversionException,
                InvalidContentFormat("meta (version 1) read invalid"));
        FWD_NESTED(done = (CheckedConverter::toUint32(metaJson["done"], 0) != 0),
                ConversionException,
                InvalidContentFormat("meta (version 1) done invalid"));

        message_->setMetaVersion(metaVersion);
        message_->setState(Dao::MessageState::Read, read);
        message_->setState(Dao::MessageState::Done, done);

        /* syncing delivery is disabled for now
        parseDelivery(metaJson["delivery"]);
        */
        break;

    default:
        // fallback for unknown meta version: use sensible defaults
        kulloAssert(metaVersion > LATEST_META_VERSION);
        message_->setMetaVersion(metaVersion);
        message_->setState(Dao::MessageState::Read, true);
        message_->setState(Dao::MessageState::Done, true);
        break;
    }
}

void MessageDecoder::parseDelivery(const Json::Value &delivery)
{
    if (delivery.isNull()) return;
    if (!delivery.isObject())
    {
        throw InvalidContentFormat("delivery is not an object");
    }
    std::vector<Delivery> parsed;
    for (const auto &recipientAddr : delivery.getMemberNames())
    {
        const auto &stateRecord = delivery[recipientAddr];
        if (!stateRecord.isObject())
        {
            throw InvalidContentFormat("stateRecord is not an object");
        }

        try
        {
            KulloAddress recipient(recipientAddr);
            auto state = Delivery::toState(CheckedConverter::toString(stateRecord["state"]));
            Delivery del(recipient, state);

            Json::Value lockRecord;
            switch (state)
            {
            case Delivery::delivered:
                del.date = CheckedConverter::toDateTime(stateRecord["date"]);
                break;
            case Delivery::failed:
                del.date = CheckedConverter::toDateTime(stateRecord["date"]);
                del.reason = Delivery::toReason(CheckedConverter::toString(stateRecord["reason"]));
                break;
            case Delivery::unsent:
                lockRecord = stateRecord["lock"];
                if (lockRecord != Json::nullValue)
                {
                    del.lockHolder = CheckedConverter::toString(lockRecord["holder"]);
                    del.lockExpires = CheckedConverter::toDateTime(lockRecord["expires"]);
                }
                break;
            default:
                // if we reach this branch, Delivery::toState() handles states that we don't know of
                kulloAssert(false);
            }
            parsed.emplace_back(del);
        }
        catch (...)
        {
            std::throw_with_nested(InvalidContentFormat("failed to parse stateRecord"));
        }
    }
    std::sort(parsed.begin(), parsed.end());
    delivery_ = std::move(parsed);
}

}
}
