/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>

#include "kulloclient/protocol/baseclient.h"

namespace Kullo {
namespace Protocol {

class AccountClient : public BaseClient
{
public:
    AccountClient(
            const Util::KulloAddress &address,
            const Util::MasterKey &masterKey);

    std::string getSettingsLocation();
};

}
}
