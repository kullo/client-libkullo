/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/protocol/pushclient.h"

#include <boost/optional/optional_io.hpp>
#include <jsoncpp/jsoncpp.h>
#include "kulloclient/protocol/debug.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/urlencoding.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Protocol {

namespace {

std::string environmentToJsonValue(Api::PushTokenEnvironment env)
{
    switch (env)
    {
    case Api::PushTokenEnvironment::Android: return "android";
    case Api::PushTokenEnvironment::IOS: return "ios";
    default: kulloAssert(false);
    }

    // never reached
    return "";
}

}

PushClient::PushClient(
        const KulloAddress &address,
        const MasterKey &masterKey,
        const std::shared_ptr<Http::HttpClient> &httpClient)
    : BaseClient(httpClient)
{
    setKulloAddress(address);
    setMasterKey(masterKey);
}

void PushClient::addGcmRegistrationToken(const Api::PushToken &token)
{
    Json::Value jsonBody(Json::objectValue);
    jsonBody["registrationToken"] = token.token;
    jsonBody["environment"] = environmentToJsonValue(token.environment);

    auto result = sendRequest(Http::Request(
                    Http::HttpMethod::Post,
                    baseUserUrl() + "/push/gcm",
                    makeHeaders(Authenticated::True)),
                jsonBody);
    Log.d() << "addGcmRegistrationToken: HTTP " << result.statusCode
            << ", error: " << result.error;
}

void PushClient::removeGcmRegistrationToken(const Api::PushToken &token)
{
    auto encodedToken = Util::UrlEncoding::encode(token.token);
    auto result = sendRequest(Http::Request(
                    Http::HttpMethod::Delete,
                    baseUserUrl() + "/push/gcm/" + encodedToken,
                    makeHeaders(Authenticated::True)));
    Log.d() << "removeGcmRegistrationToken: HTTP " << result.statusCode
            << ", error: " << result.error;
}

}
}
