/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>
#include <string>

#include "kulloclient/util/assert.h"
#include "kulloclient/util/usersettings.h"

namespace SmartSqlite {
class Connection;
}

namespace Kullo {
namespace Db {

struct SessionConfig {
    std::string dbFileName;
    Util::Credentials credentials;

    SessionConfig(
            const std::string &dbFileName,
            const Util::Credentials &credentials)
        : dbFileName(dbFileName)
        , credentials(credentials)
    {
        kulloAssert(!dbFileName.empty());
    }
};

typedef std::shared_ptr<SmartSqlite::Connection> SharedSessionPtr;

SharedSessionPtr makeSession(const SessionConfig &config);

void migrate(SharedSessionPtr session);

bool hasCurrentSchema(SharedSessionPtr session);

}
}
