/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <jsoncpp/jsoncpp-forwards.h>
#include <vector>

#include "kulloclient/protocol/baseclient.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Protocol {

class ProfileClient : public BaseClient
{
public:
    /**
     * @brief Creates a new ProfileClient.
     * @param address Address of the local user
     */
    ProfileClient(
            const Util::KulloAddress &address,
            const Util::MasterKey &masterKey,
            const std::shared_ptr<Http::HttpClient> &httpClient);

    std::vector<ProfileInfo> downloadChanges(lastModified_type modifiedAfter);
    ProfileInfo uploadChange(ProfileInfo newValue);

private:
    static ProfileInfo parseJsonProfileInfo(const Json::Value &json);
    static ProfileInfo parseJsonKeyLastModified(const Json::Value &json);
};

}
}
