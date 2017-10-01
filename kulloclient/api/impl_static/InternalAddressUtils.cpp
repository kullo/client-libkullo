/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/InternalAddressUtils.h"

#include <stdexcept>

#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Api {

bool InternalAddressUtils::isValid(const std::string &localPart, const std::string &domainPart)
{
    try {
        auto inString = localPart + "#" + domainPart;

        // test that arguments are a valid address
        auto address = Util::KulloAddress(inString);

        // test that arguments are normalized
        return address.toString() == inString;
    }
    catch(std::invalid_argument)
    {
        return false;
    }
}

}
}
