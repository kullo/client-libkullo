/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/protocol/accountclient.h"

#include <jsoncpp/jsoncpp.h>

#include "kulloclient/http/HttpClient.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/checkedconverter.h"

namespace Kullo {
namespace Protocol {

AccountClient::AccountClient(
        const Util::KulloAddress &address,
        const Util::MasterKey &masterKey)
{
    setKulloAddress(address);
    setMasterKey(masterKey);
}

std::string AccountClient::getSettingsLocation()
{
    sendRequest(Http::Request(
                    Http::HttpMethod::Get,
                    baseUserUrl() + "/account/info",
                    makeHeaders(Authenticated::True)));

    auto json = parseJsonBody();

    if (!json.isObject())
    {
        throw ProtocolError("Malformed JSON document (object expected)");
    }

    auto jsonSettingsLocation = json["settingsLocation"];
    if (jsonSettingsLocation.isNull())
    {
        // Field `settingsLocation` not set. This is valid.
        // Not used at the moment.
        return "";
    }
    else
    {
        return Util::CheckedConverter::toString(json["settingsLocation"]);
    }
}

}
}
