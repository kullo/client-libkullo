/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/protocol/messagesclient.h"

#include <boost/optional.hpp>
#include <jsoncpp/jsoncpp.h>

#include "kulloclient/http/HttpMethod.h"
#include "kulloclient/http/Request.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/base64.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/mimemultipart.h"
#include "kulloclient/util/version.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Protocol {

namespace {
// This timeout limits the entire download or send process. When sending 100 MiB via
// 1 MBit/s this takes 14 minutes.
const std::int32_t ATTACHMENT_REQUEST_TIMEOUT_MS = 60*60*1000; // 60 minutes
}

MessagesClient::MessagesClient(
        const KulloAddress &address,
        const MasterKey &masterKey,
        const std::shared_ptr<Http::HttpClient> &httpClient)
    : BaseClient(httpClient)
{
    setKulloAddress(address);
    setMasterKey(masterKey);
}

std::size_t MessagesClient::sendMessage(
        const KulloAddress &recipient,
        const SendableMessage &message,
        const ProgressHandler &onProgress)
{
    auto result = doSendMessage(
                &recipient,
                message,
                std::vector<unsigned char>(),
                onProgress);
    return result.requestBodySize;
}

MessageSent MessagesClient::sendMessageToSelf(
        const SendableMessage &message,
        const std::vector<unsigned char> &meta,
        const Protocol::ProgressHandler &onProgress)
{
    auto result = doSendMessage(nullptr, message, meta, onProgress);
    kulloAssert(result.messageSent);
    return *(result.messageSent);
}

MessagesClient::GetMessagesResult MessagesClient::getMessages(
        lastModified_type modifiedAfter)
{
    auto url = baseUserUrl()
            + "/messages?modifiedAfter="
            + std::to_string(modifiedAfter)
            + "&includeData=true";

    sendRequest(Http::Request(
                    Http::HttpMethod::Get,
                    url,
                    makeHeaders(Authenticated::True)));

    auto json = parseJsonBody();
    if (!json.isObject())
    {
        throw ProtocolError("Malformed JSON document (object expected)");
    }
    auto resultsTotal = CheckedConverter::toUint32(json["resultsTotal"]);
    auto resultsReturned = CheckedConverter::toUint32(json["resultsReturned"]);

    auto data = json["data"];
    if (!data.isArray())
    {
        throw ProtocolError("Malformed JSON document (array expected)");
    }

    auto messages = std::vector<Message>();
    messages.reserve(data.size());
    for (const auto &msgJson : data)
    {
        try
        {
            messages.push_back(parseJsonMessage(msgJson));
        }
        catch (ConversionException)
        {
            std::throw_with_nested(ProtocolError("Failed to parse messages array"));
        }
    }

    GetMessagesResult result;
    result.messages = std::move(messages);
    result.countReturned = resultsReturned;
    result.countLeft = resultsTotal;
    return result;
}

MessageAttachments MessagesClient::getMessageAttachments(
        id_type id, const Protocol::ProgressHandler &onProgress)
{
    auto url = baseUserUrl()
            + "/messages/"
            + std::to_string(id)
            + "/attachments";

    sendRequest(Http::Request(
                    Http::HttpMethod::Get,
                    url,
                    makeHeaders(Authenticated::True)),
                onProgress,
                ATTACHMENT_REQUEST_TIMEOUT_MS);

    MessageAttachments attachments;
    attachments.id = id;
    attachments.attachments = responseBody_;
    responseBody_.clear();
    return attachments;
}

IdLastModified MessagesClient::modifyMeta(
        const IdLastModified &idlm,
        const std::vector<unsigned char> &meta)
{
    auto url = baseUserUrl()
            + "/messages/"
            + std::to_string(idlm.id)
            + "?lastModified="
            + std::to_string(idlm.lastModified);

    Json::Value jsonBody(Json::objectValue);
    jsonBody["meta"] = Base64::encode(meta);

    sendRequest(Http::Request(
                    Http::HttpMethod::Patch,
                    url,
                    makeHeaders(Authenticated::True)),
                jsonBody);

    auto json = parseJsonBody();
    try
    {
        return parseJsonIdLastModified(json);
    }
    catch (ConversionException)
    {
        std::throw_with_nested(ProtocolError("Failed to parse idLastModified"));
    }
}

IdLastModified MessagesClient::deleteMessage(
        const IdLastModified &idlm)
{
    auto url = baseUserUrl()
            + "/messages/"
            + std::to_string(idlm.id)
            + "?lastModified="
            + std::to_string(idlm.lastModified);

    sendRequest(Http::Request(
                    Http::HttpMethod::Delete,
                    url,
                    makeHeaders(Authenticated::True)));

    auto json = parseJsonBody();
    try
    {
        return parseJsonIdLastModified(json);
    }
    catch (ConversionException)
    {
        std::throw_with_nested(ProtocolError("Failed to parse idLastModified"));
    }
}

MessagesClient::SendMessageResult MessagesClient::doSendMessage(
        const KulloAddress *recipient,
        const SendableMessage &message,
        const std::vector<unsigned char> &meta,
        const boost::optional<ProgressHandler> &onProgress)
{
    bool toSelf = !recipient;
    kulloAssert((!toSelf && meta.empty()) || (toSelf && !meta.empty()));

    auto auth = (toSelf) ? Authenticated::True : Authenticated::False;

    Util::MimeMultipart multipart;
    multipart.addPart("keySafe", message.keySafe);
    multipart.addPart("content", message.content);
    if (!meta.empty()) multipart.addPart("meta", meta);
    multipart.addPart("attachments", message.attachments);

    auto contentType = std::string("multipart/form-data; boundary=\"")
            + multipart.boundary()
            + "\"";
    auto body = multipart.toString();

    sendRequest(Http::Request(
                    Http::HttpMethod::Post,
                    baseUserUrl(recipient) + "/messages",
                    makeHeaders(auth, contentType, body.size())),
                body,
                onProgress,
                ATTACHMENT_REQUEST_TIMEOUT_MS);

    SendMessageResult result;
    if (toSelf)
    {
        auto json = parseJsonBody();
        try
        {
            result.messageSent = parseJsonMessageSent(json);
            result.messageSent->size = body.size();
            result.requestBodySize = body.size();
        }
        catch (ConversionException)
        {
            std::throw_with_nested(
                        ProtocolError("Failed to parse message sent data"));
        }
    }
    return result;
}

Message MessagesClient::parseJsonMessage(const Json::Value &json)
{
    if (!json.isObject())
    {
        throw ProtocolError("Malformed JSON document (object expected)");
    }
    Message msg;
    msg.id = CheckedConverter::toUint32(json["id"]);
    msg.lastModified = CheckedConverter::toUint64(json["lastModified"]);
    msg.deleted = CheckedConverter::toBool(json["deleted"]);
    if (!msg.deleted)
    {
        msg.dateReceived = CheckedConverter::toDateTime(json["dateReceived"]);
        msg.keySafe      = CheckedConverter::toVector(json["keySafe"]);
        msg.content      = CheckedConverter::toVector(json["content"]);
    }
    msg.meta = CheckedConverter::toVector(json["meta"], AllowEmpty::True);
    msg.hasAttachments = CheckedConverter::toBool(json["hasAttachments"]);
    return msg;
}

MessageSent MessagesClient::parseJsonMessageSent(const Json::Value &json)
{
    if (!json.isObject())
    {
        throw ProtocolError("Malformed JSON document (object expected)");
    }
    auto dateReceived = CheckedConverter::toDateTime(json["dateReceived"]);
    MessageSent msgSent{dateReceived};
    msgSent.id = CheckedConverter::toUint32(json["id"]);
    msgSent.lastModified = CheckedConverter::toUint64(json["lastModified"]);
    return msgSent;
}

IdLastModified MessagesClient::parseJsonIdLastModified(const Json::Value &json)
{
    if (!json.isObject())
    {
        throw ProtocolError("Malformed JSON document (object expected)");
    }
    IdLastModified idlm;
    idlm.id = CheckedConverter::toUint32(json["id"]);
    idlm.lastModified = CheckedConverter::toUint64(json["lastModified"]);
    return idlm;
}

}
}
