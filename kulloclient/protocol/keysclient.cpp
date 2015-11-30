/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/protocol/keysclient.h"

#include <jsoncpp/jsoncpp.h>

#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/base64.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/version.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Protocol {

KeysClient::KeysClient(const KulloAddress &address,
                       const MasterKey &masterKey)
{
    setKulloAddress(address);
    setMasterKey(masterKey);
}

Kullo::Protocol::SymmetricKeys KeysClient::getSymmKeys()
{
    sendRequest(Http::Request(
                    Http::HttpMethod::Get,
                    baseUserUrl() + "/keys/symm",
                    makeHeaders(Authenticated::True)));

    auto json = parseJsonBody();
    if (!json.isObject())
    {
        throw ProtocolError("Malformed JSON document (object expected)");
    }

    try
    {
        return parseJsonSymmKeys(json);
    }
    catch (ConversionException)
    {
        std::throw_with_nested(ProtocolError("Failed to parse symmKeys"));
    }
}

void KeysClient::modifySymmKeys(const SymmetricKeys &symmKeys)
{
    Json::Value jsonBody(Json::objectValue);
    jsonBody["loginKey"]       = symmKeys.loginKey;
    jsonBody["privateDataKey"] = Util::Base64::encode(symmKeys.privateDataKey);

    sendRequest(Http::Request(
                    Http::HttpMethod::Put,
                    baseUserUrl() + "/keys/symm",
                    makeHeaders(Authenticated::True)),
                jsonBody);
}

std::vector<Kullo::Protocol::KeyPair> KeysClient::getAsymmKeyPairs()
{
    sendRequest(Http::Request(
                    Http::HttpMethod::Get,
                    baseUserUrl() + "/keys/private",
                    makeHeaders(Authenticated::True)));

    auto json = parseJsonBody();
    if (!json.isArray())
    {
        throw ProtocolError("Malformed JSON document (array expected)");
    }

    std::vector<KeyPair> keyPairs;
    for (const auto &keypairJsonObject : json)
    {
        if (!keypairJsonObject.isObject())
        {
            throw ProtocolError("Malformed JSON document (object expected)");
        }
        try
        {
            keyPairs.push_back(parseJsonKeyPair(keypairJsonObject));
        }
        catch (ConversionException)
        {
            std::throw_with_nested(ProtocolError("Failed to parse keyPair"));
        }
    }
    return keyPairs;
}

SymmetricKeys KeysClient::parseJsonSymmKeys(const Json::Value &jsonObject)
{
    kulloAssert(jsonObject.isObject());
    SymmetricKeys symmKeys;
    symmKeys.privateDataKey = CheckedConverter::toVector(jsonObject["privateDataKey"]);
    return symmKeys;
}

KeyPair KeysClient::parseJsonKeyPair(const Json::Value &jsonObject)
{
    kulloAssert(jsonObject.isObject());
    KeyPair kp;
    kp.id         = CheckedConverter::toUint32(jsonObject["id"]);
    kp.type       = CheckedConverter::toString(jsonObject["type"]);
    kp.pubkey     = CheckedConverter::toVector(jsonObject["pubkey"]);
    kp.privkey    = CheckedConverter::toVector(jsonObject["privkey"], AllowEmpty::True);
    kp.validFrom  = CheckedConverter::toDateTime(jsonObject["validFrom"]);
    kp.validUntil = CheckedConverter::toDateTime(jsonObject["validUntil"]);
    kp.revocation = CheckedConverter::toVector(jsonObject["revocation"], AllowEmpty::True);
    return kp;
}

id_type KeysClient::parseJsonId(const Json::Value &jsonObject)
{
    kulloAssert(jsonObject.isObject());
    return CheckedConverter::toUint32(jsonObject["id"]);
}

}
}
