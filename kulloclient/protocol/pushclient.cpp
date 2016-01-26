/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/protocol/pushclient.h"

#include <boost/optional/optional_io.hpp>
#include <jsoncpp/jsoncpp.h>
#include "kulloclient/protocol/debug.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/urlencoding.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Protocol {

PushClient::PushClient(const KulloAddress &address, const MasterKey &masterKey)
{
    setKulloAddress(address);
    setMasterKey(masterKey);
}

void PushClient::addGcmRegistrationToken(const std::string &token)
{
    Json::Value jsonBody(Json::objectValue);
    jsonBody["registrationToken"] = token;

    auto result = sendRequest(Http::Request(
                    Http::HttpMethod::Post,
                    baseUserUrl() + "/push/gcm",
                    makeHeaders(Authenticated::True)),
                jsonBody);
    Log.d() << "addGcmRegistrationToken: HTTP " << result.statusCode
            << ", error: " << result.error;
}

void PushClient::removeGcmRegistrationToken(const std::string &token)
{
    auto encodedToken = Util::UrlEncoding::encode(token);
    auto result = sendRequest(Http::Request(
                    Http::HttpMethod::Delete,
                    baseUserUrl() + "/push/gcm/" + encodedToken,
                    makeHeaders(Authenticated::True)));
    Log.d() << "removeGcmRegistrationToken: HTTP " << result.statusCode
            << ", error: " << result.error;
}

}
}
