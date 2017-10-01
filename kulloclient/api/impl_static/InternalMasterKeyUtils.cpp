/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/InternalMasterKeyUtils.h"

#include <stdexcept>

#include "kulloclient/util/masterkey.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Api {

bool InternalMasterKeyUtils::isValid(const std::vector<std::string> &blocks)
{
    try {
        Util::MasterKey{blocks};
        return true;
    }
    catch (std::invalid_argument)
    {
        return false;
    }
}

}
}
