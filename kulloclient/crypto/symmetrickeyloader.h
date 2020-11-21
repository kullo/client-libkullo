/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/util/masterkey.h"

namespace Kullo {
namespace Crypto {

class SymmetricKeyLoader
{
public:
    static SymmetricKey fromVector(const std::vector<unsigned char> &vec);
    static SymmetricKey fromMasterKey(const Util::MasterKey &key);
};

}
}
