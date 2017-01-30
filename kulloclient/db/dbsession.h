/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
