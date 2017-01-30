/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <vector>
#include <jsoncpp/jsoncpp.h>

#include "kulloclient/codec/codecstructs.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/dao/draftdao.h"
#include "kulloclient/dao/messagedao.h"
#include "kulloclient/dao/participantdao.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/util/delivery.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/util/usersettings.h"

namespace Kullo {
namespace Codec {

/// Prepares messages for transmission
class MessageEncoder
{
public:
    /**
     * @brief Encodes a draft for sending.
     * @param settings Settings for the local user.
     * @param draft The draft to be encoded.
     * @throw IntegrityException if the data structures from DB fail the integrity check.
     */
    static EncodedMessage encodeMessage(
            const Util::Credentials &credentials,
            const Dao::DraftDao &draft,
            const Util::DateTime &dateSent,
            Db::SharedSessionPtr session);
    static EncodedMessage encodeMessage(
            const Dao::MessageDao &messageDao,
            Db::SharedSessionPtr session);

    static std::vector<unsigned char> encodeMeta(
            bool read,
            bool done,
            const std::vector<Util::Delivery> &delivery);

private:
    static EncodedMessage encodeMessage(
            id_type conversationId,
            const Dao::ParticipantDao &sender,
            const std::vector<unsigned char> &avatar,
            const std::string &dateSent,
            const std::string &text,
            const std::string &footer,
            std::unique_ptr<Dao::AttachmentResult> attachments,
            Db::SharedSessionPtr session);

    static Json::Value encodeSender(
            const Dao::ParticipantDao &sender,
            const std::vector<unsigned char> &avatar);

    static Json::Value encodeAttachments(
            EncodedMessage &msg,
            std::unique_ptr<Dao::AttachmentResult> attachments);
};

}
}
