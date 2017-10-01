/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
