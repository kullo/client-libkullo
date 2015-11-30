/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/protocol/accountsclient.h"

#include <jsoncpp/jsoncpp.h>

#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/base64.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Protocol {

boost::optional<Challenge> AccountsClient::registerAccount(
        const Util::KulloAddress &address,
        const SymmetricKeys &symmKeys,
        const KeyPair &encKeys,
        const KeyPair &sigKeys,
        const boost::optional<Challenge> &challenge,
        const boost::optional<std::string> &challengeAnswer)
{
    auto reqJson = makeRegistrationDocument(
                address, symmKeys, encKeys, sigKeys,
                challenge, challengeAnswer);

    try
    {
        sendRequest(Http::Request(
                        Http::HttpMethod::Post,
                        baseUrl(&address) + "/accounts",
                        makeHeaders(Authenticated::False)),
                    reqJson);
    }
    catch (Conflict)
    {
        throw AddressExists();
    }
    catch (Forbidden)
    {
        auto respJson = parseJsonBody();
        return boost::make_optional<Challenge>(parseJsonChallenge(respJson));
    }

    return boost::optional<Challenge>();
}

Json::Value AccountsClient::makeRegistrationDocument(
        const Util::KulloAddress &address,
        const SymmetricKeys &symmKeys,
        const KeyPair &encKeys,
        const KeyPair &sigKeys,
        const boost::optional<Challenge> &challenge,
        const boost::optional<std::string> &challengeAnswer)
{
    Json::Value keypairEncryption(Json::objectValue);
    keypairEncryption["pubkey"]  = Util::Base64::encode(encKeys.pubkey);
    keypairEncryption["privkey"] = Util::Base64::encode(encKeys.privkey);

    Json::Value keypairSigning(Json::objectValue);
    keypairSigning["pubkey"]  = Util::Base64::encode(sigKeys.pubkey);
    keypairSigning["privkey"] = Util::Base64::encode(sigKeys.privkey);

    Json::Value reg(Json::objectValue);
    reg["keypairEncryption"] = keypairEncryption;
    reg["keypairSigning"]    = keypairSigning;
    reg["address"]           = address.toString();
    reg["loginKey"]          = symmKeys.loginKey;
    reg["privateDataKey"]    = Util::Base64::encode(symmKeys.privateDataKey);

    if (challenge && challengeAnswer)
    {
        Json::Value challengeJsonObject(Json::objectValue);
        challengeJsonObject["type"]      = (*challenge).type;
        challengeJsonObject["user"]      = (*challenge).user;
        challengeJsonObject["timestamp"] = Json::UInt64((*challenge).timestamp);
        challengeJsonObject["text"]      = (*challenge).text;

        reg["challenge"]       = challengeJsonObject;
        reg["challengeAuth"]   = (*challenge).auth;
        reg["challengeAnswer"] = *challengeAnswer;
    }

    return reg;
}

Challenge AccountsClient::parseJsonChallenge(const Json::Value &contentJson)
{
    if (!contentJson.isObject())
    {
        throw ProtocolError("Malformed JSON document (object expected).");
    }

    auto challengeJson = contentJson["challenge"];
    if (!challengeJson.isObject())
    {
        throw ProtocolError("Malformed challenge in JSON document (object expected)");
    }

    Challenge challenge;
    challenge.type      = Util::CheckedConverter::toString(challengeJson["type"]);
    challenge.user      = Util::CheckedConverter::toString(challengeJson["user"]);
    challenge.timestamp = Util::CheckedConverter::toUint64(challengeJson["timestamp"]);
    challenge.text      = Util::CheckedConverter::toString(challengeJson["text"]);
    challenge.auth      = Util::CheckedConverter::toHexString(contentJson["challengeAuth"]);

    return challenge;
}

}
}
