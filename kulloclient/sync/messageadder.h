/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/types.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/dao/conversationdao.h"
#include "kulloclient/dao/messagedao.h"
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

        std::function<void(id_type conversationId, id_type messageId)>
        messageAdded;

        std::function<void(id_type conversationId, id_type messageId)>
        senderAdded;
    } events;

    void addMessage(Dao::MessageDao &message,
            Dao::ConversationDao &conversation,
            Dao::SenderDao &sender,
            const std::vector<unsigned char> &avatar,
            const std::vector<std::unique_ptr<Dao::AttachmentDao>> &attachments,
            const Db::SharedSessionPtr &session);

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
