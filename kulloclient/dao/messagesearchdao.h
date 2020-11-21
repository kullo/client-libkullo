/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/types.h"
#include "kulloclient/api/SenderPredicate.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/dao/result.h"

namespace Kullo {
namespace Dao {

struct MessageSearchDaoRow {
    int64_t msgId;
    int64_t convId;
    std::string senderAddress;
    std::string dateReceived;
    std::string snippet;
};

using MessageSearchResult = Result<MessageSearchDaoRow, MessageSearchDao>;

class MessageSearchDao
{
public:
    MessageSearchDao() = delete;

    static std::unique_ptr<MessageSearchResult> search(
            const std::string &searchText,
            id_type convId,
            const boost::optional<Api::SenderPredicate> &sender,
            int32_t limitResults,
            const std::string &boundary,
            Db::SharedSessionPtr session);

    static std::vector<std::string> tokenize(const std::string &searchText);

private:
    static std::unique_ptr<MessageSearchDaoRow> loadFromDb(
            const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    friend class Result<MessageSearchDaoRow, MessageSearchDao>;
};

}
}
