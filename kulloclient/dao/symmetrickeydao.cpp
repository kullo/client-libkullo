/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/symmetrickeydao.h"

#include "kulloclient/crypto/symmetrickeyloader.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/dao/daoutil.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Db;
using namespace Kullo::Util;

namespace Kullo {
namespace Dao {

char const *SymmetricKeyDao::MASTER_KEY_STRING = "masterKey";
char const *SymmetricKeyDao::PRIVATE_DATA_KEY_STRING = "privateDataKey";

SymmetricKeyDao::SymmetricKeyDao(SharedSessionPtr session)
    : session_(session)
{
    kulloAssert(session);
}

std::unique_ptr<SymmetricKeyDao> SymmetricKeyDao::load(SymmetricKeyDao::SymmetricKeyType type, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM keys_symm "
                "WHERE key_type = :key_type "
                "LIMIT 1");
    stmt.bind(":key_type", keyTypeToString(type));

    return SymmetricKeyResult(std::move(stmt), session).next();
}

Crypto::SymmetricKey SymmetricKeyDao::loadKey(SymmetricKeyDao::SymmetricKeyType type, SharedSessionPtr session)
{
    auto dao = load(type, session);
    if (!dao)
    {
        throw Db::DatabaseIntegrityError("Couldn't load SymmetricKeyDao of privateDataKey");
    }
    return Crypto::SymmetricKeyLoader::fromVector(dao->key());
}

bool SymmetricKeyDao::save()
{
    if (!dirty_) return false;

    std::string sql;
    if (!storedInDb_)
    {
        sql = "INSERT INTO keys_symm VALUES (:key_type, :key)";
    }
    else
    {
        sql = "UPDATE keys_symm SET key = :key WHERE key_type = :key_type";
    }
    auto stmt = session_->prepare(sql);
    stmt.bind(":key_type", keyTypeToString(keyType_));
    stmt.bind(":key", key_);
    stmt.execWithoutResult();

    storedInDb_ = true;
    dirty_ = false;
    return true;
}

SymmetricKeyDao::SymmetricKeyType SymmetricKeyDao::keyType() const
{
    return keyType_;
}

bool SymmetricKeyDao::setKeyType(SymmetricKeyDao::SymmetricKeyType keyType)
{
    if (assignAndUpdateDirty(keyType_, keyType, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

std::vector<unsigned char> SymmetricKeyDao::key() const
{
    return key_;
}

bool SymmetricKeyDao::setKey(const std::vector<unsigned char> &key)
{
    return assignAndUpdateDirty(key_, key, dirty_);
}

std::string SymmetricKeyDao::keyTypeToString(SymmetricKeyDao::SymmetricKeyType type)
{
    switch (type) {
    case MASTER_KEY:
        return MASTER_KEY_STRING;

    case PRIVATE_DATA_KEY:
        return PRIVATE_DATA_KEY_STRING;

    default:
        kulloAssertionFailed("Key type is invalid.");
    }
}

SymmetricKeyDao::SymmetricKeyType SymmetricKeyDao::stringToKeyType(const std::string &type)
{
    if (type == MASTER_KEY_STRING) return MASTER_KEY;
    if (type == PRIVATE_DATA_KEY_STRING) return PRIVATE_DATA_KEY;
    kulloAssertionFailed("Key type is invalid.");
}

std::unique_ptr<SymmetricKeyDao> SymmetricKeyDao::loadFromDb(const SmartSqlite::Row &row, SharedSessionPtr session)
{
    auto dao = make_unique<SymmetricKeyDao>(session);
    dao->keyType_ = stringToKeyType(row.get<std::string>("key_type"));
    dao->key_ = row.get<std::vector<unsigned char>>("key");
    dao->dirty_ = false;
    dao->storedInDb_ = true;
    return dao;
}

}
}
