/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/Address.h"

#include "kulloclient/api_impl/addressimpl.h"

namespace Kullo {
namespace Api {

std::shared_ptr<Address> Address::create(const std::string &address)
{
    try
    {
        return std::make_shared<ApiImpl::AddressImpl>(address);
    }
    catch (...)
    {
        return nullptr;
    }
}

}
}
