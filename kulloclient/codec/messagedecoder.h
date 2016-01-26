/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <set>
#include <vector>
#include <jsoncpp/jsoncpp-forwards.h>

#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/dao/conversationdao.h"
#include "kulloclient/dao/messagedao.h"
#include "kulloclient/dao/participantdao.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/delivery.h"
#include "kulloclient/codec/messagecompressor.h"

namespace Kullo {
namespace Codec {

enum struct VerifySignature
{
    False, True
};

/**
 * @brief Decodes a HttpClient::Message to MessageDao, AttachmentDao etc.
 *
 * All results of the parsing (conversation, message, attachments, sender)
 * are not automatically saved. You need to call save() on each of them to persist the
 * changes.
 */
class MessageDecoder
{
public:
    /**
     * @brief Construct a new decoder.
     * @param httpMessage The message to be decoded.
     * @param userAddress The local user's address.
     * @param dao The DAO where the decoded data should be stored.
     * @throw HttpClient::SyncException if there's an error while parsing.
     *
     * The instance keeps a reference for the passed httpMessage. Thus, the referenced
     * httpMessage must be valid at least until decode() and makeConversation()
     * have returned.
     *
     * A remote message to be decoded must not be deleted.
     */
    MessageDecoder(
            const DecryptedMessage &message,
            Util::KulloAddress userAddress,
            Db::SharedSessionPtr session,
            MessageCompressor *compressor = nullptr);

    /**
     * @brief Decode the message contents.
     *
     * Before calling decode, all getters but message() will return 0, and message()
     * will return a MessageDao that doesn't yet reflect the new contents of the message.
     *
     * This function must be called before makeConversation() and must not be called twice.
     */
    void decode(VerifySignature verify = VerifySignature::True);

    Dao::MessageDao &message();
    const Dao::MessageDao &message() const;

    std::vector<Util::Delivery> delivery() const;

    /**
     * @brief Create conversation for the message contents.
     *
     * Before calling this function, conversation() will return 0.
     *
     * This function must be called after decode() and must not be called twice.
     */
    void makeConversation();

    /**
     * @brief Return the Conversation for this message.
     *
     * makeConversation() must be called before this function.
     * @return 0, iff message is deleted.
     */
    Dao::ConversationDao &conversation();
    const Dao::ConversationDao &conversation() const;

    /**
     * @brief Return the parsed attachments.
     *
     * decode() must be called before this function.
     * @return List of attachments, might be empty.
     */
    std::vector<std::unique_ptr<Dao::AttachmentDao>> &attachments();

    /**
     * @brief Return the parsed sender.
     *
     * decode() must be called before this function.
     * @return 0, iff message is deleted.
     */
    Dao::ParticipantDao &sender();
    const Dao::ParticipantDao &sender() const;

    id_type signatureKeyId() const;

private:
    void parseContent(const DecryptedContent &content);
    void verifyContent(const DecryptedContent &content);
    void parseAttachments(const Json::Value &attachmentsIndex, id_type messageId);
    void parseSender(const Json::Value &sender, id_type messageId);
    void parseMeta(metaVersion_type metaVersion, const std::string &meta);
    void parseDelivery(const Json::Value &delivery);

    std::set<Util::KulloAddress> participantAddresses_;
    std::unique_ptr<MessageCompressor> compressor_;
    DecryptedMessage decryptedMessage_;
    Util::KulloAddress userAddress_;
    Db::SharedSessionPtr session_;

    std::unique_ptr<Dao::ConversationDao> conversation_;
    std::unique_ptr<Dao::MessageDao> message_;
    std::vector<Util::Delivery> delivery_;
    std::vector<std::unique_ptr<Dao::AttachmentDao>> attachments_;
    std::shared_ptr<Dao::ParticipantDao> sender_;
    bool contentDecoded_ = false;
    bool madeConversation_ = false;
};

}
}
