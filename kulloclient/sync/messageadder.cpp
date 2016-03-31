/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/sync/messageadder.h"

#include "kulloclient/util/assert.h"
#include "kulloclient/util/events.h"

namespace Kullo {
namespace Sync {

void MessageAdder::addMessage(
        Dao::MessageDao &message,
        Dao::ConversationDao &conversation,
        Dao::ParticipantDao &sender,
        const std::vector<std::unique_ptr<Dao::AttachmentDao>> &attachments)
{
    reset();

    newConversation_ = conversation.save();
    convId_ = conversation.id();
    msgId_ = message.id();
    message.setConversationId(convId_);

    sender.save();
    senderAddr_ = sender.address().toString();

    message.save(Dao::CreateOld::No);
    for (auto &att : attachments)
    {
        att->save();
    }

    addMessageHasBeenRun_ = true;
}

void MessageAdder::emitSignals()
{
    kulloAssert(addMessageHasBeenRun_);

    // sender must be available when message is added, so senderAdded must be
    // emitted before messageAdded
    EMIT(events.senderAdded, convId_, msgId_);
    EMIT(events.participantAddedOrModified, senderAddr_);
    EMIT(events.messageAdded, convId_, msgId_);

    if (newConversation_)
    {
        EMIT(events.conversationAdded, convId_);
    }
    else
    {
        EMIT(events.conversationModified, convId_);
    }
}

void MessageAdder::reset()
{
    addMessageHasBeenRun_ = false;
    newConversation_ = false;
    convId_ = 0;
    msgId_ = 0;
    senderAddr_.clear();
}

}
}
