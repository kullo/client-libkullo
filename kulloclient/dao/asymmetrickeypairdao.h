/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/crypto/privatekey.h"
#include "kulloclient/dao/result.h"
#include "kulloclient/db/dbsession.h"

namespace Kullo {
namespace Dao {

using AsymmetricKeyPairResult = Result<AsymmetricKeyPairDao>;

class AsymmetricKeyPairDao
{
public:
    static Crypto::PrivateKey loadPrivateKey(
            Crypto::AsymmetricKeyType type,
            id_type keyId,
            Db::SharedSessionPtr session);

    static Crypto::PrivateKey loadPrivateKey(
            Crypto::AsymmetricKeyType type,
            Db::SharedSessionPtr session);

    /**
     * @brief Create a new asymmetric key pair.
     */
    explicit AsymmetricKeyPairDao(Db::SharedSessionPtr session);

    /**
     * @brief Load the asymmetric key pair with the given ID from DB or create it if it doesn't exist.
     * @param id ID of the key.
     */
    static std::unique_ptr<AsymmetricKeyPairDao> load(
            Crypto::AsymmetricKeyType type,
            id_type id,
            Db::SharedSessionPtr session);

    /**
     * @brief Load the latest key pair with the given type.
     * @param type The type of key that should be retrieved.
     */
    static std::unique_ptr<AsymmetricKeyPairDao> load(
            Crypto::AsymmetricKeyType type,
            Db::SharedSessionPtr session);

    /**
     * @brief Saves the current state to DB if the state was changed.
     * @return true if state was dirty
     */
    bool save();

    id_type id() const;

    bool setId(id_type id);

    Crypto::AsymmetricKeyType keyType() const;

    bool setKeyType(Crypto::AsymmetricKeyType keyType);

    std::vector<unsigned char> pubkey() const;

    bool setPubkey(const std::vector<unsigned char> &pubkey);

    std::vector<unsigned char> privkey() const;

    bool setPrivkey(const std::vector<unsigned char> &privkey);

    std::string validFrom() const;

    bool setValidFrom(const std::string &validFrom);

    std::string validUntil() const;

    bool setValidUntil(const std::string &validUntil);

    std::vector<unsigned char> revocation() const;

    bool setRevocation(const std::vector<unsigned char> &revocation);

private:
    static std::unique_ptr<AsymmetricKeyPairDao> loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    Db::SharedSessionPtr session_;
    bool dirty_ = true, storedInDb_ = false;
    id_type id_ = 0;
    Crypto::AsymmetricKeyType keyType_ = Crypto::AsymmetricKeyType::Invalid;
    std::vector<unsigned char> pubkey_;
    std::vector<unsigned char> privkey_;
    std::vector<unsigned char> revocation_;
    std::string validFrom_, validUntil_;

    friend class Result<AsymmetricKeyPairDao>;
};

}
}
