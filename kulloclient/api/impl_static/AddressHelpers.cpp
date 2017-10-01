/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/AddressHelpers.h"

#include "kulloclient/api_impl/Address.h"
#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Api {

boost::optional<Address> AddressHelpers::create(const std::string &addressString)
{
    try
    {
        auto address = Util::KulloAddress(addressString);
        return Api::Address(address);
    }
    catch (std::invalid_argument)
    {
        return boost::none;
    }
}

}
}
