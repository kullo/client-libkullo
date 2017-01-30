/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>

#include "kulloclient/protocol/baseclient.h"

namespace Kullo {
namespace Protocol {

struct AccountInfo final {
    boost::optional<std::string> planName;
    boost::optional<std::uint64_t> storageQuota;
    boost::optional<std::uint64_t> storageUsed;
    boost::optional<std::string> settingsLocation;
};

class AccountClient : public BaseClient
{
public:
    AccountClient(
            const Util::KulloAddress &address,
            const Util::MasterKey &masterKey,
            const std::shared_ptr<Http::HttpClient> &httpClient);

    AccountInfo getInfo();
};

}
}
