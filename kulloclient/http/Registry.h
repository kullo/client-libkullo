// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http.djinni

#pragma once

#include <kulloclient/nn.h>
#include <memory>

namespace Kullo { namespace Http {

class HttpClientFactory;

class Registry {
public:
    virtual ~Registry() {}

    static void setHttpClientFactory(const ::Kullo::nn_shared_ptr<HttpClientFactory> & factory);
};

} }  // namespace Kullo::Http
