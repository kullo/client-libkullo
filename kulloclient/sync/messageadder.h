/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "kulloclient/types.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/dao/conversationdao.h"
#include "kulloclient/dao/messagedao.h"
#include "kulloclient/dao/participantdao.h"
#include "kulloclient/db/dbsession.h"

namespace Kullo {
namespace Sync {

class MessageAdder
{
public:
    struct {
        std::function<void(id_type conversationId)>
        conversationAdded;

        std::function<void(id_type conversationId)>
        conversationModified;

        std::function<void(std::string address)>
        participantAddedOrModified;

        std::function<void(id_type conversationId, id_type messageId)>
        messageAdded;

        std::function<void(id_type conversationId, id_type messageId)>
        senderAdded;
    } events;

    void addMessage(Dao::MessageDao &message,
            Dao::ConversationDao &conversation,
            Dao::ParticipantDao &sender,
            const std::vector<std::unique_ptr<Dao::AttachmentDao>> &attachments);

    void emitSignals();

private:
    void reset();

    bool addMessageHasBeenRun_ = false;
    bool newConversation_ = false;
    id_type convId_ = 0;
    id_type msgId_ = 0;
    std::string senderAddr_;
};

}
}
