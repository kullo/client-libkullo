/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
