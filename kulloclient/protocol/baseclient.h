/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <boost/optional/optional_fwd.hpp>
#include <jsoncpp/jsoncpp-forwards.h>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/http/HttpHeader.h"
#include "kulloclient/http/Response.h"

namespace Kullo {
namespace Protocol {

using ProgressHandler = std::function<void(const Http::TransferProgress &)>;

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
    static const std::int32_t DEFAULT_TIMEOUT_MS = 10*1000; // 10 seconds

    enum struct Authenticated { False, True };

    std::string baseUrl(const Util::KulloAddress *address = nullptr);
    std::string baseUserUrl(const Util::KulloAddress *address = nullptr);
    std::vector<Http::HttpHeader> makeHeaders(
            Authenticated auth,
            const std::string &contentType = "application/json",
            boost::optional<std::size_t> contentLength = boost::none);

    Http::Response sendRequest(
            const Http::Request &request,
            const boost::optional<ProgressHandler> &onProgress = boost::none,
            const std::int32_t timeout = DEFAULT_TIMEOUT_MS);
    Http::Response sendRequest(
            const Http::Request &request,
            const Json::Value &reqJson,
            const std::int32_t timeout = DEFAULT_TIMEOUT_MS);
    Http::Response sendRequest(
            const Http::Request &request,
            const std::string &reqBody,
            const boost::optional<ProgressHandler> &onProgress,
            const std::int32_t timeout = DEFAULT_TIMEOUT_MS);

    void throwOnError(const Http::Response &response);
    Json::Value parseJsonBody();

    std::unique_ptr<Util::KulloAddress> address_;
    std::unique_ptr<Util::MasterKey> masterKey_;
    std::atomic<bool> canceled_;
    std::shared_ptr<Http::HttpClient> client_;
    std::vector<uint8_t> responseBody_;

private:
    Http::Response doSendRequest(
            const Http::Request &request,
            const std::string *reqBody,
            const boost::optional<ProgressHandler> onProgress,
            const std::int32_t timeout);
};

}
}
