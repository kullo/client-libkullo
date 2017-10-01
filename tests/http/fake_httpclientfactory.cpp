/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "tests/http/fake_httpclientfactory.h"

#include <jsoncpp/jsoncpp.h>

#include <kulloclient/http/HttpHeader.h>
#include <kulloclient/http/Request.h>
#include <kulloclient/http/RequestListener.h>
#include <kulloclient/http/Response.h>
#include <kulloclient/http/ResponseListener.h>
#include <kulloclient/crypto/symmetrickeygenerator.h>
#include <kulloclient/util/assert.h>
#include <kulloclient/util/base64.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/hex.h>
#include <kulloclient/util/regex.h>

#include "tests/testdata.h"

using namespace Kullo;

namespace {
const std::string EXISTING_USER = "exists#example.com";
const std::string BLOCKED_USER = "blocked#example.com";
const std::string URL_USER_PREFIX = "https://kullo.example.com/v1/exists%23example.com";

int32_t validateAuth(const std::vector<Http::HttpHeader> headers)
{
    for (const auto &header : headers)
    {
        if (header.key == "Authorization")
        {
            const std::string prefix = "Basic ";
            if (header.value.find(prefix) != 0)
            {
                // no basic auth;
                return 401;
            }

            auto loginStringBase64 = header.value.substr(prefix.size());
            auto loginString = Util::to_string(
                        Util::Base64::decode(loginStringBase64));

            auto colonPos = loginString.find(':');
            if (colonPos == std::string::npos)
            {
                return 401;
            }

            auto username = loginString.substr(0, colonPos);
            if (username != EXISTING_USER)
            {
                return 401;
            }

            auto loginKeyHex = loginString.substr(
                        colonPos + 1,
                        loginString.size() - colonPos - 1);

            auto expected = Crypto::SymmetricKeyGenerator().makeLoginKey(
                        Util::KulloAddress(EXISTING_USER),
                        Util::MasterKey(MasterKeyData::VALID_DATA_BLOCKS));
            if (loginKeyHex != expected.toHex())
            {
                return 401;
            }

            // all tests where successful
            return 200;
        }
    }

    // no Authorization header found
    return 401;
}
}

Http::Response FakeHttpClient::sendRequest(const Kullo::Http::Request &request,
        int32_t /*timeoutMs*/,
        const std::shared_ptr<Http::RequestListener> &requestListener,
        const std::shared_ptr<Http::ResponseListener> &responseListener)
{
    boost::optional<Http::ResponseError> responseError;
    int32_t statusCode = 0;
    std::vector<uint8_t> responseBody;

    // public encryption key for existing address
    if (request.method == Http::HttpMethod::Get && request.url ==
            URL_USER_PREFIX + "/keys/public/latest-enc")
    {
        statusCode = 200;
        responseBody = Util::to_vector(
                    R"({"id": 1, "type": "enc", "pubkey": "FakePubKey", )"
                    R"("validFrom": "2015-01-01T01:01:01Z", )"
                    R"("validUntil": "2015-01-01T01:01:01Z", )"
                    R"("revocation": ""})");
    }

    // public encryption key for non-existing address
    else if (request.method == Http::HttpMethod::Get && request.url ==
             "https://kullo.example.com/v1/doesntexist%23example.com"
             "/keys/public/latest-enc")
    {
        statusCode = 404;
    }

    // private keys for existing logged in user
    else if (request.method == Http::HttpMethod::Get
             && request.url == URL_USER_PREFIX + "/keys/symm")
    {
        statusCode = 200;

        // iv_raw=$(head -c 16 /dev/urandom)
        // iv_hex=$(echo -n "$iv_raw" | xxd -ps -c 999)
        // key_raw=$(./kullomasterkey.py --decode --file=masterkey-zero.kpem)
        // key_hex=$(echo -n "$key_raw" | xxd -ps -c 999)
        // (echo -n "$iv_raw" && head -c 32 /dev/urandom | ./Botan encryption --mode=aes-256-gcm --debug --key=$key_hex --iv=$iv_hex) | base64
        //
        responseBody = Util::to_vector(
                    "{"
                    "\"privateDataKey\":"
                    "\"7mAPO6xlDsucgqV3P9vgqUaaRk02NvSdY0nYr2mjTkRzY0ifYi+KmHFuSfITg7aWiev3juGEaaoojo/QZ7GhqQ==\""
                    "}\n");
    }

    // private keys for existing logged in user
    else if (request.method == Http::HttpMethod::Get
             && request.url == URL_USER_PREFIX + "/keys/private")
    {
        statusCode = 200;
        responseBody = Util::to_vector("[]");
    }

    // empty inbox for existing address
    else if (request.method == Http::HttpMethod::Get &&
             request.url.find(URL_USER_PREFIX + "/messages") == 0)
    {
        statusCode = validateAuth(request.headers);
        if (statusCode == 200)
        {
            responseBody = Util::to_vector(
                        R"({"resultsTotal": 0, "resultsReturned": 0, )"
                        R"("data": []})");
        }
    }

    // account info
    else if (request.method == Http::HttpMethod::Get &&
             request.url == URL_USER_PREFIX + "/account/info")
    {
        statusCode = validateAuth(request.headers);
        if (statusCode == 200)
        {
            responseBody = Util::to_vector(
                        R"(  {                                                             )"
                        R"(    "planName": "Hobbyist",                                     )"
                        R"(    "storageQuota": 1000000000,                                 )"
                        R"(    "storageUsed":   999999999,                                 )"
                        R"(    "settingsLocation": "https://accounts.example.com/foo/bar"  )"
                        R"(  }                                                             )"
                        );
        }
    }

    // list user profile info
    else if (request.method == Http::HttpMethod::Get &&
             request.url.find(URL_USER_PREFIX + "/profile") == 0)
    {
        statusCode = validateAuth(request.headers);
        if (statusCode == 200)
        {
            responseBody = Util::to_vector(R"({"data": []})");
        }
    }

    // update user profile info
    else if (request.method == Http::HttpMethod::Put &&
             request.url.find(URL_USER_PREFIX + "/profile/") == 0)
    {
        statusCode = validateAuth(request.headers);
        if (statusCode == 200)
        {
            static const Util::Regex keyRegex(R"(/profile/([a-z_]+)/?)");
            std::vector<std::string> matches;
            if (!Util::Regex::search(request.url, matches, keyRegex))
            {
                kulloAssertionFailed("Regex didn't match.");
            }
            responseBody = Util::to_vector(
                        std::string(R"({"key": ")")
                        + matches[1]
                        + R"(", "lastModified": 1337})");
        }
    }

    // GCM registration token
    else if (request.method == Http::HttpMethod::Post &&
             request.url == URL_USER_PREFIX + "/push/gcm")
    {
        statusCode = validateAuth(request.headers);
        if (statusCode == 200)
        {
            auto json = readJson(requestListener->read(10*1024*1024));

            auto environment = json["environment"];
            if (environment != "android" && environment != "ios")
            {
                statusCode = 400;
            }
            else
            {
                auto token = json["registrationToken"];
                if (token == "return_http_400")
                {
                    statusCode = 400;
                }
                else if (token == "return_http_500")
                {
                    statusCode = 500;
                }
                else if (token == "return_connection_error")
                {
                    statusCode = 0;
                    responseError = Http::ResponseError::NetworkError;
                }
                else if (token == "return_timeout")
                {
                    statusCode = 0;
                    responseError = Http::ResponseError::Timeout;
                }
                else
                {
                    responseBody = Util::to_vector("{}");
                }
            }
        }
    }

    // registration
    else if (request.method == Http::HttpMethod::Post &&
             request.url == "https://kullo.example.com/v1/accounts")
    {
        auto json = readJson(
                    requestListener->read(std::numeric_limits<int64_t>::max()));

        auto address = json["address"];

        // non-existing address which doesn't need a challenge: success
        if (address == "nochallenge#example.com")
        {
            statusCode = 200;
        }

        // existing address: conflict
        else if (address == "exists#example.com")
        {
            statusCode = 409;
        }

        else if (address == "withchallenge#example.com")
        {
            const std::string TYPE = "code";
            const std::string USER = "withchallenge#example.com";
            const uint64_t TIMESTAMP = 1433768060000000ull;
            const std::string TEXT = "foofoofoo";
            const std::string AUTH = "deadbeef";
            const std::string ANSWER = "42";

            auto jsonChallenge = json["challenge"];
            if (
                    jsonChallenge["type"] == TYPE &&
                    jsonChallenge["user"] == USER &&
                    jsonChallenge["timestamp"] == Json::Value::UInt64(TIMESTAMP) &&
                    jsonChallenge["text"] == TEXT &&
                    json["challengeAuth"] == AUTH &&
                    json["challengeAnswer"] == ANSWER)
            {
                statusCode = 200;
            }
            else
            {
                statusCode = 403;
                responseBody = Util::to_vector(
                            std::string() +
                            R"({"challenge": {)"
                                R"("type": ")" + TYPE + "\", "
                                R"("user": ")" + USER + "\", "
                                R"("timestamp": )" + std::to_string(TIMESTAMP) + ", "
                                R"("text": ")" + TEXT + "\"}, "
                            R"("challengeAuth": ")" + AUTH + "\"}");
            }
        }

        else if (address == BLOCKED_USER)
        {
            const std::string TYPE = "blocked";
            const std::string USER = "blockeduser#example.com";
            const uint64_t TIMESTAMP = 1433768060000000ull;
            const std::string TEXT = "foofoofoo";
            const std::string AUTH = "deadbeef";

            // "blocked" challenge cannot be solved => always send challenge
            statusCode = 403;
            responseBody = Util::to_vector(
                        R"({"challenge": {)"
                            R"("type": ")" + TYPE + "\", "
                            R"("user": ")" + USER + "\", "
                            R"("timestamp": )" + std::to_string(TIMESTAMP) + ", "
                            R"("text": ")" + TEXT + "\"}, "
                        R"("challengeAuth": ")" + AUTH + "\"}");
        }

        else
        {
            kulloAssertionFailed("Unknown address");
        }
    }

    else
    {
        // If we get here, we received a request that isn't handled (yet)
        kulloAssertionFailed("Request type not implemented");
    }

    if (responseListener) responseListener->dataReceived(responseBody);
    return Http::Response(
                responseError,
                statusCode,
                std::vector<Http::HttpHeader>());
}

Json::Value FakeHttpClient::readJson(const std::vector<uint8_t> &responseBody)
{
    auto respBodyData = reinterpret_cast<const char*>(responseBody.data());
    Json::Value json;
    Json::Reader(Json::Features::strictMode()).parse(
                respBodyData,
                respBodyData + responseBody.size(),
                json,
                false);
    return json;
}

Kullo::nn_shared_ptr<Http::HttpClient> FakeHttpClientFactory::createHttpClient()
{
    return Kullo::nn_make_shared<FakeHttpClient>();
}

std::unordered_map<std::string, std::string> FakeHttpClientFactory::versions()
{
    return std::unordered_map<std::string, std::string>();
}
