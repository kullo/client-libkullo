/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <memory>
#include <jsoncpp/jsoncpp-forwards.h>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/http/HttpClient.h"
#include "kulloclient/http/HttpHeader.h"
#include "kulloclient/http/Response.h"

namespace Kullo {
namespace Protocol {

class BaseClient
{
public:
    BaseClient(const std::shared_ptr<Http::HttpClient> &httpClient);
    virtual ~BaseClient();

    void setKulloAddress(const Util::KulloAddress &address);
    void setMasterKey(const Util::MasterKey &masterKey);

    // thread safe
    void cancel();

protected:
    enum struct Authenticated { False, True };

    std::string baseUrl(const Util::KulloAddress *address = nullptr);
    std::string baseUserUrl(const Util::KulloAddress *address = nullptr);
    std::vector<Http::HttpHeader> makeHeaders(
            Authenticated auth,
            const std::string &contentType = "application/json");

    Http::Response sendRequest(const Http::Request &request);
    Http::Response sendRequest(
            const Http::Request &request, const Json::Value &reqJson);
    Http::Response sendRequest(
            const Http::Request &request, const std::string &reqBody);

    void throwOnError(const Http::Response &response);
    Json::Value parseJsonBody();

    std::unique_ptr<Util::KulloAddress> address_;
    std::unique_ptr<Util::MasterKey> masterKey_;
    std::atomic<bool> canceled_;
    std::shared_ptr<Http::HttpClient> client_;
    std::vector<uint8_t> responseBody_;

private:
    Http::Response doSendRequest(
            const Http::Request &request, const std::string *reqBody);
};

}
}
