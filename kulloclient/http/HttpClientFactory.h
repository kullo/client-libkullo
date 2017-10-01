// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http.djinni

#pragma once

#include <kulloclient/nn.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace Kullo { namespace Http {

class HttpClient;

class HttpClientFactory {
public:
    virtual ~HttpClientFactory() {}

    virtual ::Kullo::nn_shared_ptr<HttpClient> createHttpClient() = 0;

    /** Returns pairs of <library name, version number> for the libraries used. */
    virtual std::unordered_map<std::string, std::string> versions() = 0;
};

} }  // namespace Kullo::Http
