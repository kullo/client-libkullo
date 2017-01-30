/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/dao/result.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Dao {

typedef Result<PublicKeyDao> PublicKeyResult;

class PublicKeyDao
{
public:
    enum PublicKeyType {INVALID, ENCRYPTION, SIGNATURE};
    static char const *ENCRYPTION_STRING;
    static char const *SIGNATURE_STRING;

    explicit PublicKeyDao(const Util::KulloAddress &address, Db::SharedSessionPtr session);
    static std::unique_ptr<PublicKeyDao> load(const Util::KulloAddress &address, PublicKeyType keyType, id_type id, Db::SharedSessionPtr session);
    bool save();

    Util::KulloAddress address() const;
    bool setAddress(const Util::KulloAddress &address);

    PublicKeyType keyType() const;
    bool setKeyType(PublicKeyType keyType);

    id_type id() const;
    bool setId(id_type id);

    std::vector<unsigned char> key() const;
    bool setKey(const std::vector<unsigned char> &key);

private:
    static std::string keyTypeToString(PublicKeyType type);
    static PublicKeyType stringToKeyType(const std::string &type);
    static std::unique_ptr<PublicKeyDao> loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    Db::SharedSessionPtr session_;
    bool dirty_ = true, storedInDb_ = false;

    Util::KulloAddress address_;
    PublicKeyType keyType_ = INVALID;
    id_type id_ = 0;
    std::vector<unsigned char> key_;

    friend class Result<PublicKeyDao>;
};

}
}
