/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <jsoncpp/jsoncpp-forwards.h>

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
            const Util::MasterKey &masterKey);

    void addGcmRegistrationToken(const std::string &token);
    void removeGcmRegistrationToken(const std::string &token);
};

}
}
