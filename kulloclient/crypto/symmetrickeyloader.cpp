/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
