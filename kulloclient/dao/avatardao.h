/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/db/dbsession.h"

namespace Kullo {
namespace Dao {

/**
 * @brief Data Access Object for avatars
 */
class AvatarDao
{
public:
    static std::vector<unsigned char> load(std::int64_t hash, const Db::SharedSessionPtr &session);
    static std::map<std::int64_t, std::vector<unsigned char>> all(const Db::SharedSessionPtr &session);
    static std::int64_t store(const std::vector<unsigned char> &avatar, const Db::SharedSessionPtr &session);
};

}
}
