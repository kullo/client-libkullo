/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
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
