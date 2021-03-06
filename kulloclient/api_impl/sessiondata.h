/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/api_impl/usersettingsimpl.h"
#include "kulloclient/codec/privatekeyprovider.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

struct SessionData {
    const Db::SessionConfig sessionConfig_;
    const Db::SharedSessionPtr dbSession_;
    const std::shared_ptr<UserSettingsImpl> userSettings_;
    const std::shared_ptr<Codec::PrivateKeyProvider> privKeyProvider_;

    SessionData(
            const Db::SessionConfig &sessionConfig,
            Db::SharedSessionPtr dbSession,
            std::shared_ptr<UserSettingsImpl> userSettings)
        : sessionConfig_(sessionConfig)
        , dbSession_(dbSession)
        , userSettings_(userSettings)
        , privKeyProvider_(
              std::make_shared<Codec::PrivateKeyProvider>(sessionConfig_))
    {
        kulloAssert(dbSession_);
        kulloAssert(userSettings_);
    }
};

}
}
