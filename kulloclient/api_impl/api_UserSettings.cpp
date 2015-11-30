/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/UserSettings.h"

#include "kulloclient/api_impl/usersettingsimpl.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Api {

std::shared_ptr<UserSettings> UserSettings::create(
        const std::shared_ptr<Address> &address,
        const std::shared_ptr<MasterKey> &masterKey)
{
    kulloAssert(address);
    kulloAssert(masterKey);
    return std::make_shared<ApiImpl::UserSettingsImpl>(address, masterKey);
}

}
}
