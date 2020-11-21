/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/MasterKey.h"

#include "kulloclient/util/masterkey.h"

namespace Kullo {
namespace Api {

MasterKey::MasterKey(const Util::MasterKey &masterKey)
    : MasterKeyBase(masterKey.dataBlocks())
{
}

MasterKey::MasterKey(const std::vector<std::string> &blocks)
    : MasterKey(Util::MasterKey(blocks))
{
}

bool MasterKey::operator==(const MasterKey &rhs) const
{
    return blocks == rhs.blocks;
}

}
}
