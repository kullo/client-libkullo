/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/avatardao.h"

#include <smartsqlite/connection.h>

#include "kulloclient/crypto/hasher.h"

using namespace Kullo::Db;
using namespace Kullo::Util;

namespace Kullo {
namespace Dao {

std::vector<unsigned char> AvatarDao::load(int64_t hash, const SharedSessionPtr &session)
{
    auto stmt = session->prepare("SELECT avatar FROM avatars WHERE hash=:hash");
    stmt.bind(":hash", hash);
    auto row = stmt.execWithSingleResult();
    return row.get<std::vector<unsigned char>>("avatar");
}

std::map<std::int64_t, std::vector<unsigned char>> AvatarDao::all(const SharedSessionPtr &session)
{
    std::map<std::int64_t, std::vector<unsigned char>> result;

    auto stmt = session->prepare("SELECT hash, avatar FROM avatars");
    for (auto row : stmt)
    {
        auto hash = row.get<std::int64_t>("hash");
        auto avatar = row.get<std::vector<unsigned char>>("avatar");
        result.emplace(hash, avatar);
    }

    return result;
}

int64_t AvatarDao::store(const std::vector<unsigned char> &avatar, const SharedSessionPtr &session)
{
    auto hash = Crypto::Hasher::eightByteHash(avatar);

    auto avatarStmt = session->prepare(
                "INSERT OR IGNORE INTO avatars (hash, avatar) "
                "VALUES (:hash, :avatar)");
    avatarStmt.bind(":hash", hash);
    avatarStmt.bind(":avatar", avatar);
    avatarStmt.execWithoutResult();

    return hash;
}

}
}
