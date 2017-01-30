/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/dao/result.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Dao {

typedef Result<UnprocessedMessageDao> UnprocessedMessageResult;

class UnprocessedMessageDao
{
public:
    explicit UnprocessedMessageDao(const Util::KulloAddress &sender, Db::SharedSessionPtr session);
    static std::unique_ptr<UnprocessedMessageDao> load(id_type id, Db::SharedSessionPtr session);
    static std::unique_ptr<UnprocessedMessageDao> loadFirst(Db::SharedSessionPtr session);
    static std::unique_ptr<UnprocessedMessageDao> loadFirstForSignatureKey(const Util::KulloAddress &address, id_type sigKeyId, Db::SharedSessionPtr session);
    static void deleteAllForSignatureKey(const Util::KulloAddress &address, id_type sigKeyId, Db::SharedSessionPtr session);
    bool save();
    void deletePermanently();

    Util::KulloAddress sender() const;
    bool setSender(const Util::KulloAddress &sender);

    id_type signatureKeyId() const;
    bool setSignatureKeyId(id_type sigKeyId);

    Protocol::Message message() const;
    bool setMessage(const Protocol::Message &msg);

private:
    static std::unique_ptr<UnprocessedMessageDao> loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    Db::SharedSessionPtr session_;
    bool dirty_ = true, storedInDb_ = false;

    Util::KulloAddress sender_;
    id_type sigKeyId_ = 0;
    Protocol::Message msg_;

    friend class Result<UnprocessedMessageDao>;
};

}
}
