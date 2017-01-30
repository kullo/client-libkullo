/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/optional/optional_fwd.hpp>
#include <jsoncpp/jsoncpp-forwards.h>
#include <vector>
#include <cstdint>

#include "kulloclient/protocol/baseclient.h"
#include "kulloclient/protocol/httpstructs.h"

namespace Kullo {
namespace Protocol {

/**
 * @brief The HTTP client for sending, retrieving and modifying messages.
 *
 * All requests are executed asynchronously.
 */
class MessagesClient : public BaseClient
{
public:
    /**
     * @brief Creates a new MessagesClient.
     * @param address Address of the local user
     */
    MessagesClient(
            const Util::KulloAddress &address,
            const Util::MasterKey &masterKey,
            const std::shared_ptr<Http::HttpClient> &httpClient);

    /**
     * @brief Sends a message.
     * @param recipient Recipient's address
     * @param message Message record to be sent
     */
    std::size_t sendMessage(
            const Util::KulloAddress &recipient,
            const SendableMessage &message,
            const ProgressHandler &onProgress);

    MessageSent sendMessageToSelf(
            const SendableMessage &message,
            const std::vector<unsigned char> &meta,
            const ProgressHandler &onProgress);

    struct GetMessagesResult
    {
        std::vector<Message> messages;
        std::uint32_t countReturned;
        std::uint32_t countLeft;
    };

    /**
     * @brief Get a list of messages from the server.
     * @param modifiedAfter Only messages with a modification timestamp larger than the given value are returned
     */
     GetMessagesResult getMessages(lastModified_type modifiedAfter);

    /**
     * @brief Get a single message with the given ID from the server.
     */
    Message getMessage(id_type id);

    /**
     * @brief Get the attachments of a single message from the server.
     */
    MessageAttachments getMessageAttachments(
            id_type id, const ProgressHandler &onProgress);

    /**
     * @brief Update the message meta for the message identified by idlm.
     * @param meta The binary, encrypted, non-base64-encoded meta data
     *
     * This will succeed only if idlm matches the information on the server.
     * Otherwise, it will return a conflict.
     */
    IdLastModified modifyMeta(
            const IdLastModified &idlm,
            const std::vector<unsigned char> &meta);

    /**
     * @brief Delete the message identified by idlm.
     *
     * This will succeed only if idlm matches the information on the server.
     * Otherwise, it will return a conflict.
     */
    IdLastModified deleteMessage(const IdLastModified &idlm);

private:
    struct SendMessageResult
    {
        boost::optional<MessageSent> messageSent;
        std::size_t requestBodySize;
    };

    SendMessageResult doSendMessage(
            const Util::KulloAddress *recipient,
            const SendableMessage &message,
            const std::vector<unsigned char> &meta,
            const boost::optional<ProgressHandler> &onProgress = boost::none);

    static Message parseJsonMessage(const Json::Value &json);
    static MessageSent parseJsonMessageSent(const Json::Value &json);
    static IdLastModified parseJsonIdLastModified(const Json::Value &json);
};

}
}
