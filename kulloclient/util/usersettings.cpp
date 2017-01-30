/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/usersettings.h"

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Util {

UserSettings::UserSettings(const Credentials &credentials)
    : credentials(credentials)
{}

Credentials::Credentials(
        std::shared_ptr<KulloAddress> &&address,
        std::shared_ptr<MasterKey> &&masterKey)
    : address(std::move(address))
    , masterKey(std::move(masterKey))
{
    kulloAssert(this->address);
    kulloAssert(this->masterKey);
}

}
}
