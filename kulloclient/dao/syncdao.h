/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/dao/result.h"
#include "kulloclient/db/dbsession.h"

namespace Kullo {
namespace Dao {

class SyncDao
{
public:
    enum DataType {
        Message,
        Profile,
    };

    static lastModified_type timestamp(DataType type, Db::SharedSessionPtr session);
    static void setTimestamp(DataType type, lastModified_type timestamp, Db::SharedSessionPtr session);

private:
    static std::string dataTypeToString(DataType type);
};

}
}
