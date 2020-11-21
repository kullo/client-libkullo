/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <vector>
#include <jsoncpp/jsoncpp-forwards.h>

#include <kulloclient/http/HttpClient.h>
#include <kulloclient/http/HttpClientFactory.h>

class FakeHttpClient : public Kullo::Http::HttpClient
{
public:
    Kullo::Http::Response sendRequest(
            const Kullo::Http::Request &request,
            int32_t timeoutMs,
            const std::shared_ptr<Kullo::Http::RequestListener> &requestListener,
            const std::shared_ptr<Kullo::Http::ResponseListener> &responseListener
            ) override;

private:
    static Json::Value readJson(const std::vector<uint8_t> &responseBody);
};

class FakeHttpClientFactory : public Kullo::Http::HttpClientFactory
{
public:
    Kullo::nn_shared_ptr<Kullo::Http::HttpClient> createHttpClient() override;
    std::unordered_map<std::string, std::string> versions() override;
};
