/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/crypto/symmetrickey.h"
#include "kulloclient/dao/result.h"
#include "kulloclient/db/dbsession.h"

namespace Kullo {
namespace Dao {

typedef Result<SymmetricKeyDao> SymmetricKeyResult;

class SymmetricKeyDao
{
public:
    enum SymmetricKeyType {INVALID, MASTER_KEY, PRIVATE_DATA_KEY};
    static char const *MASTER_KEY_STRING;
    static char const *PRIVATE_DATA_KEY_STRING;

    /**
     * @brief Create a new symmetric key.
     */
    explicit SymmetricKeyDao(Db::SharedSessionPtr session);

    /**
     * @brief Load the latest key pair with the given type.
     * @param type The type of key that should be retrieved.
     */
    static std::unique_ptr<SymmetricKeyDao> load(SymmetricKeyType type, Db::SharedSessionPtr session);

    static Crypto::SymmetricKey loadKey(SymmetricKeyType type, Db::SharedSessionPtr session);

    /**
     * @brief Saves the current state to DB if the state was changed.
     * @return true if state was dirty
     */
    bool save();

    SymmetricKeyType keyType() const;
    bool setKeyType(SymmetricKeyType keyType);

    std::vector<unsigned char> key() const;

    bool setKey(const std::vector<unsigned char> &key);

private:
    static std::string keyTypeToString(SymmetricKeyType type);
    static SymmetricKeyType stringToKeyType(const std::string &type);
    static std::unique_ptr<SymmetricKeyDao> loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    Db::SharedSessionPtr session_;
    bool dirty_ = true, storedInDb_ = false;
    SymmetricKeyType keyType_ = INVALID;
    std::vector<unsigned char> key_;

    friend class Result<SymmetricKeyDao>;
};

}
}
