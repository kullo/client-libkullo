/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/asymmetrickeypairdao.h"

#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/dao/daoutil.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Db;
using namespace Kullo::Util;

namespace Kullo {
namespace Dao {

const std::string AsymmetricKeyPairDao::ENCRYPTION_STRING = "enc";
const std::string AsymmetricKeyPairDao::SIGNATURE_STRING = "sig";

Crypto::PrivateKey AsymmetricKeyPairDao::loadPrivateKey(
        Crypto::AsymmetricKeyType type,
        id_type keyId,
        Db::SharedSessionPtr session)
{
    auto keyPairDao = AsymmetricKeyPairDao::load(type, keyId, session);
    if (!keyPairDao)
    {
        throw DatabaseIntegrityError(std::string("Couldn't load AsymmetricKeyPairDao with id ")
                                     + std::to_string(keyId));
    }

    Crypto::AsymmetricKeyLoader loader;
    return loader.loadUnencryptedPrivateKey(type, keyPairDao->privkey());
}

Crypto::PrivateKey AsymmetricKeyPairDao::loadPrivateKey(Crypto::AsymmetricKeyType type, SharedSessionPtr session)
{
    auto keyPairDao = AsymmetricKeyPairDao::load(type, session);
    if (!keyPairDao)
    {
        throw DatabaseIntegrityError("Couldn't load keyPairDao");
    }

    Crypto::AsymmetricKeyLoader loader;
    return loader.loadUnencryptedPrivateKey(type, keyPairDao->privkey());
}

AsymmetricKeyPairDao::AsymmetricKeyPairDao(SharedSessionPtr session)
    : session_(session)
{
    kulloAssert(session);
}

std::unique_ptr<AsymmetricKeyPairDao> AsymmetricKeyPairDao::load(Crypto::AsymmetricKeyType type, id_type id, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM keys_asymm WHERE id = :id AND key_type = :key_type");
    stmt.bind(":id", id);
    stmt.bind(":key_type", keyTypeToString(type));

    return AsymmetricKeyPairResult(std::move(stmt), session).next();
}

std::unique_ptr<AsymmetricKeyPairDao> AsymmetricKeyPairDao::load(Crypto::AsymmetricKeyType type, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM keys_asymm "
                "WHERE key_type = :key_type "
                    "AND datetime(valid_from) <= datetime('now') "
                    "AND datetime(valid_until) >= datetime('now') "
                "ORDER BY datetime(valid_from) DESC");
    stmt.bind(":key_type", keyTypeToString(type));

    return AsymmetricKeyPairResult(std::move(stmt), session).next();
}

bool AsymmetricKeyPairDao::save()
{
    if (!dirty_) return false;

    std::string sql;
    if (!storedInDb_)
    {
        sql = "INSERT INTO keys_asymm VALUES ("
                ":id, :key_type, :pubkey, :privkey, "
                ":valid_from, :valid_until, :revocation)";
    }
    else
    {
        sql = "UPDATE keys_asymm "
                "SET key_type = :key_type, pubkey = :pubkey, privkey = :privkey, "
                "valid_from = :valid_from, valid_until = :valid_until, revocation = :revocation "
                "WHERE id = :id";
    }
    auto stmt = session_->prepare(sql);
    stmt.bind(":id", id_);
    stmt.bind(":key_type", keyTypeToString(keyType_));
    stmt.bind(":pubkey", pubkey_);
    stmt.bind(":privkey", privkey_);
    stmt.bind(":valid_from", validFrom_);
    stmt.bind(":valid_until", validUntil_);
    stmt.bind(":revocation", revocation_);
    stmt.execWithoutResult();

    storedInDb_ = true;
    dirty_ = false;
    return true;
}

id_type AsymmetricKeyPairDao::id() const
{
    return id_;
}

bool AsymmetricKeyPairDao::setId(id_type id)
{
    if (assignAndUpdateDirty(id_, id, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

Crypto::AsymmetricKeyType AsymmetricKeyPairDao::keyType() const
{
    return keyType_;
}

bool AsymmetricKeyPairDao::setKeyType(Crypto::AsymmetricKeyType keyType)
{
    return assignAndUpdateDirty(keyType_, keyType, dirty_);
}

std::vector<unsigned char> AsymmetricKeyPairDao::pubkey() const
{
    return pubkey_;
}

bool AsymmetricKeyPairDao::setPubkey(const std::vector<unsigned char> &pubkey)
{
    return assignAndUpdateDirty(pubkey_, pubkey, dirty_);
}

std::vector<unsigned char> AsymmetricKeyPairDao::privkey() const
{
    return privkey_;
}

bool AsymmetricKeyPairDao::setPrivkey(const std::vector<unsigned char> &privkey)
{
    return assignAndUpdateDirty(privkey_, privkey, dirty_);
}

std::string AsymmetricKeyPairDao::validFrom() const
{
    return validFrom_;
}

bool AsymmetricKeyPairDao::setValidFrom(const std::string &validFrom)
{
    return assignAndUpdateDirty(validFrom_, validFrom, dirty_);
}

std::string AsymmetricKeyPairDao::validUntil() const
{
    return validUntil_;
}

bool AsymmetricKeyPairDao::setValidUntil(const std::string &validUntil)
{
    return assignAndUpdateDirty(validUntil_, validUntil, dirty_);
}

std::vector<unsigned char> AsymmetricKeyPairDao::revocation() const
{
    return revocation_;
}

bool AsymmetricKeyPairDao::setRevocation(const std::vector<unsigned char> &revocation)
{
    return assignAndUpdateDirty(revocation_, revocation, dirty_);
}

std::string AsymmetricKeyPairDao::keyTypeToString(Crypto::AsymmetricKeyType type)
{
    switch (type) {
    case Crypto::AsymmetricKeyType::Encryption:
        return ENCRYPTION_STRING;

    case Crypto::AsymmetricKeyType::Signature:
        return SIGNATURE_STRING;

    default:
        kulloAssert(false);
        return "";
    }
}

Crypto::AsymmetricKeyType AsymmetricKeyPairDao::stringToKeyType(const std::string &type)
{
    if (type == ENCRYPTION_STRING) return Crypto::AsymmetricKeyType::Encryption;
    if (type == SIGNATURE_STRING) return Crypto::AsymmetricKeyType::Signature;
    kulloAssert(false);
    return Crypto::AsymmetricKeyType::Invalid;
}

std::unique_ptr<AsymmetricKeyPairDao> AsymmetricKeyPairDao::loadFromDb(const SmartSqlite::Row &row, SharedSessionPtr session)
{
    auto dao = make_unique<AsymmetricKeyPairDao>(session);
    dao->id_ = row.get<id_type>("id");
    dao->keyType_ = stringToKeyType(row.get<std::string>("key_type"));
    dao->pubkey_ = row.get<std::vector<unsigned char>>("pubkey");
    dao->privkey_ = row.get<std::vector<unsigned char>>("privkey");
    dao->validFrom_ = row.get<std::string>("valid_from");
    dao->validUntil_ = row.get<std::string>("valid_until");
    dao->revocation_ = row.get<std::vector<unsigned char>>("revocation");
    dao->dirty_ = false;
    dao->storedInDb_ = true;
    return dao;
}

}
}
