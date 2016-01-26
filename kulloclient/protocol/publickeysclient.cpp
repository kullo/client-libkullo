/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/protocol/publickeysclient.h"

#include <limits>
#include <jsoncpp/jsoncpp.h>

#include "kulloclient/http/HttpMethod.h"
#include "kulloclient/http/Request.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/version.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Protocol {

const id_type PublicKeysClient::LATEST_ENCRYPTION_PUBKEY =
        std::numeric_limits<id_type>::max();

Kullo::Protocol::KeyPair PublicKeysClient::getPublicKey(
        const KulloAddress &username, id_type id)
{
    std::string idStr;
    if (id == LATEST_ENCRYPTION_PUBKEY)
    {
        idStr = "latest-enc";
    }
    else
    {
        idStr = std::to_string(id);
    }

    sendRequest(Http::Request(
                    Http::HttpMethod::Get,
                    baseUserUrl(&username) + "/keys/public/" + idStr,
                    makeHeaders(Authenticated::False)));

    auto json = parseJsonBody();
    if (!json.isObject())
    {
        throw ProtocolError("Malformed JSON document (object expected)");
    }

    try
    {
        return parseJsonKeyPair(json);
    }
    catch (...)
    {
        std::throw_with_nested(ProtocolError("Failed to parse keyPair"));
    }
}

KeyPair PublicKeysClient::parseJsonKeyPair(const Json::Value &jsonObject)
{
    kulloAssert(jsonObject.isObject());
    KeyPair kp;
    FWD_NESTED(kp.id = CheckedConverter::toUint32(jsonObject["id"]),
            ConversionException, ProtocolError("id invalid"));
    FWD_NESTED(kp.type = CheckedConverter::toString(jsonObject["type"]),
            ConversionException, ProtocolError("type invalid"));
    FWD_NESTED(kp.pubkey = CheckedConverter::toVector(jsonObject["pubkey"]),
            ConversionException, ProtocolError("pubkey invalid"));
    FWD_NESTED(kp.validFrom = CheckedConverter::toDateTime(jsonObject["validFrom"]),
            ConversionException, ProtocolError("validFrom invalid"));
    FWD_NESTED(kp.validUntil = CheckedConverter::toDateTime(jsonObject["validUntil"]),
            ConversionException, ProtocolError("validUntil invalid"));
    FWD_NESTED(kp.revocation = CheckedConverter::toVector(jsonObject["revocation"], AllowEmpty::True),
            ConversionException, ProtocolError("revocation invalid"));
    return kp;
}

}
}
