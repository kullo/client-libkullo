/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <string>

namespace SmartSqlite {
class Connection;
}

namespace Kullo {
namespace Db {

typedef std::shared_ptr<SmartSqlite::Connection> SharedSessionPtr;

SharedSessionPtr makeSession(const std::string &dbFileName);

void migrate(SharedSessionPtr session);

bool hasCurrentSchema(SharedSessionPtr session);

}
}
