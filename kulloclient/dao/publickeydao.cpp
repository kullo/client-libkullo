/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/dao/publickeydao.h"

#include "kulloclient/dao/daoutil.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Dao {

char const *PublicKeyDao::ENCRYPTION_STRING = "enc";
char const *PublicKeyDao::SIGNATURE_STRING = "sig";

PublicKeyDao::PublicKeyDao(const Util::KulloAddress &address, Db::SharedSessionPtr session)
    : session_(session),
      address_(address)
{
    kulloAssert(session);
}

std::unique_ptr<PublicKeyDao> PublicKeyDao::load(const Util::KulloAddress &address, PublicKeyDao::PublicKeyType keyType, id_type id, Db::SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM keys_public "
                "WHERE address = :address AND key_type = :key_type AND id = :id");
    stmt.bind(":address", address.toString());
    stmt.bind(":key_type", keyTypeToString(keyType));
    stmt.bind(":id", id);

    return PublicKeyResult(std::move(stmt), session).next();
}

bool PublicKeyDao::save()
{
    if (!dirty_) return false;

    std::string sql;
    if (!storedInDb_)
    {
        sql = "INSERT INTO keys_public VALUES ("
              ":address, :key_type, :id, :key)";
    }
    else
    {
        sql = "UPDATE keys_public "
              "SET key = :key "
              "WHERE address = :address AND key_type = :key_type AND id = :id";
    }
    auto stmt = session_->prepare(sql);
    stmt.bind(":address", address_.toString());
    stmt.bind(":key_type", keyTypeToString(keyType_));
    stmt.bind(":id", id_);
    stmt.bind(":key", key_);
    stmt.execWithoutResult();

    storedInDb_ = true;
    dirty_ = false;
    return true;
}

Util::KulloAddress PublicKeyDao::address() const
{
    return address_;
}

bool PublicKeyDao::setAddress(const Util::KulloAddress &address)
{
    if (assignAndUpdateDirty(address_, address, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

PublicKeyDao::PublicKeyType PublicKeyDao::keyType() const
{
    return keyType_;
}

bool PublicKeyDao::setKeyType(PublicKeyDao::PublicKeyType keyType)
{
    if (assignAndUpdateDirty(keyType_, keyType, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

id_type PublicKeyDao::id() const
{
    return id_;
}

bool PublicKeyDao::setId(id_type id)
{
    if (assignAndUpdateDirty(id_, id, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

std::vector<unsigned char> PublicKeyDao::key() const
{
    return key_;
}

bool PublicKeyDao::setKey(const std::vector<unsigned char> &key)
{
    return assignAndUpdateDirty(key_, key, dirty_);
}

std::string PublicKeyDao::keyTypeToString(PublicKeyDao::PublicKeyType type)
{
    switch (type) {
    case ENCRYPTION:
        return ENCRYPTION_STRING;

    case SIGNATURE:
        return SIGNATURE_STRING;

    default:
        kulloAssertionFailed("Key type is invalid.");
    }
}

PublicKeyDao::PublicKeyType PublicKeyDao::stringToKeyType(const std::string &type)
{
    if (type == ENCRYPTION_STRING) return ENCRYPTION;
    if (type == SIGNATURE_STRING) return SIGNATURE;
    kulloAssertionFailed("Key type is invalid.");
}

std::unique_ptr<PublicKeyDao> PublicKeyDao::loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session)
{
    auto address = Util::KulloAddress(row.get<std::string>("address"));
    auto dao = make_unique<PublicKeyDao>(address, session);
    dao->keyType_ = stringToKeyType(row.get<std::string>("key_type"));
    dao->id_ = row.get<id_type>("id");
    dao->key_ = row.get<std::vector<unsigned char>>("key");
    dao->dirty_ = false;
    dao->storedInDb_ = true;
    return dao;
}

}
}
