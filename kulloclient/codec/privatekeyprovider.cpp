/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/codec/privatekeyprovider.h"

#include "kulloclient/dao/asymmetrickeypairdao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/scopedbenchmark.h"

namespace Kullo {
namespace Codec {

PrivateKeyProvider::PrivateKeyProvider(const Db::SessionConfig &sessionConfig)
    : session_(Db::makeSession(sessionConfig))
{
}

Crypto::PrivateKey PrivateKeyProvider::getKey(
        Crypto::AsymmetricKeyType type,
        id_type keyId)
{
    auto benchmark = Util::ScopedBenchmark(
                "Get key " + std::to_string(keyId) + " " + toString(type));
    K_RAII(benchmark);

    // A reader-writer lock would allow concurrent reads, but we shouldn't
    // currently encounter any concurrency at all, so this is fine as it
    // guarantees thread safety.
    std::lock_guard<std::mutex> cacheLock(cacheMutex_);
    K_RAII(cacheLock);

    auto key = std::make_tuple(type, keyId);
    auto cacheIter = cache_.find(key);
    if (cacheIter == cache_.end())
    {
        auto result = Dao::AsymmetricKeyPairDao::loadPrivateKey(
                    type, keyId, session_);
        cache_.insert(std::make_pair(key, result));
        return result;
    }
    else
    {
        return cacheIter->second;
    }
}

std::pair<Crypto::PrivateKey, id_type> PrivateKeyProvider::getLatestKeyAndId(
        Crypto::AsymmetricKeyType type)
{
    auto dao = Dao::AsymmetricKeyPairDao::load(type, session_);
    if (!dao)
    {
        throw Db::DatabaseIntegrityError("Couldn't load latest asymm key");
    }
    return std::pair<Crypto::PrivateKey, id_type>(
                getKey(type, dao->id()),
                dao->id());
}

}
}
