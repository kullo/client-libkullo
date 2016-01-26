/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <map>
#include <mutex>
#include <tuple>
#include <utility>

#include "kulloclient/crypto/asymmetrickeytype.h"
#include "kulloclient/crypto/privatekey.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Codec {

/**
 * @brief Caches PrivateKey instances
 *
 * Loading asymmmetric keys is expensive, so we want to cache the results.
 */
class PrivateKeyProvider
{
public:
    PrivateKeyProvider(const std::string &dbPath);

    /// Get key with the given type and ID from cache or DB. Thread-safe.
    virtual Crypto::PrivateKey getKey(
            Crypto::AsymmetricKeyType type,
            id_type keyId);

    /// Get most recent key with the given type from cache or DB. Thread-safe.
    virtual std::pair<Crypto::PrivateKey, id_type> getLatestKeyAndId(
            Crypto::AsymmetricKeyType type);

private:
    const Db::SharedSessionPtr session_;

    std::mutex cacheMutex_;
    using TypeIdTuple = std::tuple<Crypto::AsymmetricKeyType, id_type>;
    std::map<TypeIdTuple, Crypto::PrivateKey> cache_;
};

}
}
