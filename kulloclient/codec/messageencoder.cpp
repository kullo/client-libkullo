/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/codec/messageencoder.h"

#include <jsoncpp/jsoncpp.h>

#include "kulloclient/codec/exceptions.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/dao/avatardao.h"
#include "kulloclient/dao/conversationdao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/base64.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/jsonhelper.h"
#include "kulloclient/util/limits.h"

using namespace Kullo::Dao;
using namespace Kullo::Util;

namespace Kullo {
namespace Codec {

EncodedMessage MessageEncoder::encodeMessage(
        const Util::Credentials &credentials,
        const Dao::DraftDao &draft,
        const Util::DateTime &dateSent,
        Db::SharedSessionPtr session)
{
    auto sender = Dao::ParticipantDao(*credentials.address, session);
    sender.setName(draft.senderName());
    sender.setOrganization(draft.senderOrganization());
    sender.setAvatarMimeType(draft.senderAvatarMimeType());

    auto attachments = AttachmentDao::allForMessage(
                IsDraft::Yes, draft.conversationId(), session);

    return encodeMessage(
                draft.conversationId(),
                sender,
                draft.senderAvatar(),
                dateSent.toString(),
                draft.text(),
                draft.footer(),
                std::move(attachments),
                session);
}

EncodedMessage MessageEncoder::encodeMessage(
        const MessageDao &messageDao, Db::SharedSessionPtr session)
{
    auto sender = Dao::ParticipantDao::load(messageDao.id(), session);
    if (!sender)
    {
        throw Db::DatabaseIntegrityError(
                    "MessageEncoder::encodeMessage(): "
                    "couldn't find sender for message");
    }
    std::vector<unsigned char> avatar;
    if (sender->avatarHash())
    {
        avatar = Dao::AvatarDao::load(*sender->avatarHash(), session);
    }
    auto attachments = AttachmentDao::allForMessage(
                IsDraft::No, messageDao.id(), session);

    return encodeMessage(
                messageDao.conversationId(),
                *sender,
                avatar,
                messageDao.dateSent(),
                messageDao.text(),
                messageDao.footer(),
                std::move(attachments),
                session);
}

std::vector<unsigned char> MessageEncoder::encodeMeta(
        bool read,
        bool done,
        const std::vector<Delivery> &delivery)
{
    Json::Value metaObj(Json::objectValue);
    metaObj["read"] = read ? 1 : 0;
    metaObj["done"] = done ? 1 : 0;

    if (!delivery.empty())
    {
        /* syncing delivery is disabled for now
        Json::Value deliveryObj(Json::objectValue);
        for (const auto &del : delivery)
        {
            Json::Value recipientObj(Json::objectValue);
            recipientObj["state"] = Delivery::toString(del.state);
            switch (del.state)
            {
            case Delivery::delivered:
                recipientObj["date"] = del.date->toString();
                break;
            case Delivery::failed:
                recipientObj["date"] = del.date->toString();
                recipientObj["reason"] = Delivery::toString(*del.reason);
                break;
            case Delivery::unsent:
                if (del.lockHolder && del.lockExpires)
                {
                    Json::Value lockObj(Json::objectValue);
                    lockObj["holder"] = *del.lockHolder;
                    lockObj["expires"] = del.lockExpires->toString();
                    recipientObj["lock"] = std::move(lockObj);
                }
                break;
            default:
                kulloAssert(false);
            }
            deliveryObj[del.recipient.toString()] = std::move(recipientObj);
        }
        metaObj["delivery"] = std::move(deliveryObj);
    */
    }
    return Util::to_vector(Json::FastWriter().write(metaObj));
}

EncodedMessage MessageEncoder::encodeMessage(
        id_type conversationId,
        const ParticipantDao &sender,
        const std::vector<unsigned char> &avatar,
        const std::string &dateSent,
        const std::string &text,
        const std::string &footer,
        std::unique_ptr<AttachmentResult> attachments,
        Db::SharedSessionPtr session)
{
    EncodedMessage message;

    Json::Value messageContentJson(Json::objectValue);
    messageContentJson["sender"] = encodeSender(sender, avatar);

    auto conv = ConversationDao::load(conversationId, session);
    if (!conv)
    {
        throw Db::DatabaseIntegrityError(
                    "MessageEncoder::MessageEncoder(): "
                    "couldn't find conversation for draft");
    }
    messageContentJson["recipients"] =
            JsonHelper::stringsToJsonArray(conv->participantsList());
    messageContentJson["dateSent"] = dateSent;
    messageContentJson["text"] = text;
    messageContentJson["footer"] = footer;

    messageContentJson["attachmentsIndex"] =
            encodeAttachments(message, std::move(attachments));

    message.content = to_vector(Json::FastWriter().write(messageContentJson));
    return message;
}

Json::Value MessageEncoder::encodeSender(
        const Dao::ParticipantDao &sender,
        const std::vector<unsigned char> &avatar)
{
    Json::Value senderJson(Json::objectValue);
    senderJson["address"]      = sender.address().toString();
    senderJson["name"]         = sender.name();
    senderJson["organization"] = sender.organization();

    Json::Value avatarJson(Json::objectValue);
    if (!(sender.avatarMimeType().empty() || avatar.empty()))
    {
        avatarJson["mimeType"] = sender.avatarMimeType();
        avatarJson["data"]     = Base64::encode(avatar);
    }
    senderJson["avatar"] = avatarJson;

    return senderJson;
}

Json::Value MessageEncoder::encodeAttachments(
        EncodedMessage &msg,
        std::unique_ptr<AttachmentResult> attachments)
{
    Json::Value attIndex(Json::arrayValue);

    std::vector<unsigned char> data;
    while (auto dao = attachments->next())
    {
        std::vector<unsigned char> content = dao->content();
        if (dao->size() != content.size())
        {
            throw Db::DatabaseIntegrityError(
                        "MessageEncoder: attachment sizes don't match");
        }
        std::copy(content.cbegin(), content.cend(), std::back_inserter(data));

        Json::Value idxEle(Json::objectValue);
        idxEle["filename"] = dao->filename();
        idxEle["mimeType"] = dao->mimeType();
        idxEle["note"] = dao->note();
        idxEle["size"] = Json::Value::UInt(dao->size());
        idxEle["hash"] = dao->hash();
        attIndex.append(idxEle);
    }
    msg.attachments.swap(data);
    return attIndex;
}

}
}
