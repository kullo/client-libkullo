/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/symmetrickeyloader.h"

#include "kulloclient/crypto/symmetrickey.h"
#include "kulloclient/util/hex.h"

namespace Kullo {
namespace Crypto {

SymmetricKey SymmetricKeyLoader::fromVector(const std::vector<unsigned char> &vec)
{
    return SymmetricKey(Util::Hex::encode(vec));
}

SymmetricKey SymmetricKeyLoader::fromMasterKey(const Util::MasterKey &key)
{
    return fromVector(key.toVector());
}

}
}
