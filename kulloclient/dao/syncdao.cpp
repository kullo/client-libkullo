/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/syncdao.h"

#include <smartsqlite/exceptions.h>

namespace Kullo {
namespace Dao {

lastModified_type SyncDao::timestamp(DataType type, Db::SharedSessionPtr session)
{
    auto stmt = session->prepare(
                "SELECT timestamp FROM sync "
                "WHERE type = :type");
    stmt.bind(":type", dataTypeToString(type));

    try
    {
        return stmt.execWithSingleResult().get<lastModified_type>(0);
    }
    catch (SmartSqlite::QueryReturnedNoRows)
    {
        return 0;
    }
}

void SyncDao::setTimestamp(SyncDao::DataType type, lastModified_type timestamp, Db::SharedSessionPtr session)
{
    auto stmt = session->prepare(
                "INSERT OR REPLACE INTO sync "
                "(type, timestamp) "
                "VALUES (:type, :timestamp)");
    stmt.bind(":type", dataTypeToString(type));
    stmt.bind(":timestamp", timestamp);
    stmt.execWithoutResult();
}

std::string SyncDao::dataTypeToString(SyncDao::DataType type)
{
    switch (type)
    {
    case Message: return "messages";
    case Profile: return "usersettings";

    default:
        kulloAssert(false);
        return "";
    }
}

}
}
