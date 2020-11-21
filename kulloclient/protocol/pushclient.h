/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <jsoncpp/jsoncpp-forwards.h>

#include "kulloclient/api/PushToken.h"
#include "kulloclient/protocol/baseclient.h"

namespace Kullo {
namespace Protocol {

class PushClient : public BaseClient
{
public:
    /**
     * @brief Creates a new PushClient.
     * @param address Address of the local user
     */
    PushClient(
            const Util::KulloAddress &address,
            const Util::MasterKey &masterKey,
            const std::shared_ptr<Http::HttpClient> &httpClient);

    void addGcmRegistrationToken(const Api::PushToken &token);
    void removeGcmRegistrationToken(const Api::PushToken &token);
};

}
}
