/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/protocol/profileclient.h"

#include <jsoncpp/jsoncpp.h>

#include "kulloclient/http/Request.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/librarylogger.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Protocol {

ProfileClient::ProfileClient(
        const KulloAddress &address,
        const MasterKey &masterKey,
        const std::shared_ptr<Http::HttpClient> &httpClient)
    : BaseClient(httpClient)
{
    setKulloAddress(address);
    setMasterKey(masterKey);
}

std::vector<ProfileInfo> ProfileClient::downloadChanges(
        lastModified_type modifiedAfter)
{
    sendRequest(Http::Request(
                    Http::HttpMethod::Get,
                    baseUserUrl()
                    + "/profile?modifiedAfter="
                    + std::to_string(modifiedAfter),
                    makeHeaders(Authenticated::True)));

    auto json = parseJsonBody();
    if (!json.isObject())
    {
        throw ProtocolError("Malformed JSON document (object expected)");
    }

    auto data = json["data"];
    if (!data.isArray())
    {
        throw ProtocolError("Malformed JSON document (array expected)");
    }

    auto result = std::vector<ProfileInfo>();
    result.reserve(data.size());
    for (const auto &profileInfoJson : data)
    {
        try
        {
            result.push_back(parseJsonProfileInfo(profileInfoJson));
        }
        catch (ConversionException)
        {
            std::throw_with_nested(
                        ProtocolError("Failed to parse profile info array"));
        }
    }
    return result;
}

ProfileInfo ProfileClient::uploadChange(ProfileInfo newValue)
{
    auto url = baseUserUrl()
            + "/profile/"
            + newValue.key
            + "?lastModified="
            + std::to_string(newValue.lastModified);

    Json::Value jsonBody(Json::objectValue);
    jsonBody["value"] = newValue.value;

    sendRequest(Http::Request(
                    Http::HttpMethod::Put,
                    url,
                    makeHeaders(Authenticated::True)),
                jsonBody);

    auto json = parseJsonBody();
    try
    {
        auto result = parseJsonKeyLastModified(json);
        if (result.key != newValue.key)
        {
            throw ProtocolError("Server sent the wrong record");
        }
        result.value = newValue.value;
        return result;
    }
    catch (ConversionException)
    {
        std::throw_with_nested(ProtocolError("Failed to parse body"));
    }
}

ProfileInfo ProfileClient::parseJsonProfileInfo(const Json::Value &json)
{
    auto result = parseJsonKeyLastModified(json);
    result.value = CheckedConverter::toString(json["value"]);
    return result;
}

ProfileInfo ProfileClient::parseJsonKeyLastModified(const Json::Value &json)
{
    if (!json.isObject())
    {
        throw ProtocolError("Malformed JSON document (object expected)");
    }
    ProfileInfo result;
    result.key = CheckedConverter::toString(json["key"]);
    result.lastModified = CheckedConverter::toUint64(json["lastModified"]);
    return result;
}

}
}
