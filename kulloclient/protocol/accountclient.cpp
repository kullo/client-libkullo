/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/protocol/accountclient.h"

#include <jsoncpp/jsoncpp.h>

#include "kulloclient/http/HttpClient.h"
#include "kulloclient/http/HttpMethod.h"
#include "kulloclient/http/Request.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/checkedconverter.h"

namespace Kullo {
namespace Protocol {

AccountClient::AccountClient(
        const Util::KulloAddress &address,
        const Util::MasterKey &masterKey,
        const std::shared_ptr<Http::HttpClient> &httpClient)
    : BaseClient(httpClient)
{
    setKulloAddress(address);
    setMasterKey(masterKey);
}

AccountInfo AccountClient::getInfo()
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

    auto jsonPlanName = json["planName"];
    auto jsonStorageQuota = json["storageQuota"];
    auto jsonStorageUsed = json["storageUsed"];
    auto jsonSettingsLocation = json["settingsLocation"];

    AccountInfo out;

    if (!jsonPlanName.isNull())
    {
        out.planName = Util::CheckedConverter::toString(jsonPlanName);
    }

    if (!jsonStorageQuota.isNull())
    {
        out.storageQuota = Util::CheckedConverter::toUint64(jsonStorageQuota);
    }

    if (!jsonStorageUsed.isNull())
    {
        out.storageUsed = Util::CheckedConverter::toUint64(jsonStorageUsed);
    }

    // Field `settingsLocation` may be not set. This is valid. Not used at the moment.
    if (!jsonSettingsLocation.isNull())
    {
        out.settingsLocation = Util::CheckedConverter::toString(jsonSettingsLocation);
    }

    return out;
}

}
}
