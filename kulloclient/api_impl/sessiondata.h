/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api_impl/usersettingsimpl.h"
#include "kulloclient/codec/privatekeyprovider.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

struct SessionData {
    const std::string dbPath_;
    const Db::SharedSessionPtr dbSession_;
    const std::shared_ptr<UserSettingsImpl> userSettings_;
    const std::shared_ptr<Codec::PrivateKeyProvider> privKeyProvider_;

    SessionData(
            const std::string &dbPath,
            Db::SharedSessionPtr dbSession,
            std::shared_ptr<UserSettingsImpl> userSettings)
        : dbPath_(dbPath)
        , dbSession_(dbSession)
        , userSettings_(userSettings)
        , privKeyProvider_(
              std::make_shared<Codec::PrivateKeyProvider>(dbPath_))
    {
        kulloAssert(!dbPath_.empty());
        kulloAssert(dbSession_);
        kulloAssert(userSettings_);
    }
};

}
}
