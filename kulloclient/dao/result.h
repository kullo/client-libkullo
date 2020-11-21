/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <cstdint>
#include <memory>

#include <smartsqlite/connection.h>
#include <smartsqlite/statement.h>

#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Dao {

template <typename T, typename Loader = T>
class Result
{
public:
    Result(SmartSqlite::Statement stmt, Db::SharedSessionPtr session)
        : m_stmt(std::move(stmt)), m_session(session), m_iter(m_stmt.begin())
    {
        kulloAssert(session);
    }

    std::unique_ptr<T> next()
    {
        if (m_iter == m_stmt.end()) return nullptr;
        std::unique_ptr<T> result(Loader::loadFromDb(*m_iter, m_session));
        ++m_iter;
        return result;
    }

private:
    SmartSqlite::Statement m_stmt;
    Db::SharedSessionPtr m_session;
    SmartSqlite::RowIterator m_iter;
};

}
}
