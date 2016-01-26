/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/MasterKey.h"

#include "kulloclient/api_impl/masterkeyimpl.h"
#include "kulloclient/util/masterkey.h"

namespace Kullo {
namespace Api {

std::shared_ptr<MasterKey> MasterKey::createFromPem(const std::string &pem)
{
    try
    {
        return std::make_shared<ApiImpl::MasterKeyImpl>(pem);
    }
    catch (...)
    {
        return nullptr;
    }
}

std::shared_ptr<MasterKey> MasterKey::createFromDataBlocks(
        const std::vector<std::string> &dataBlocks)
{
    try
    {
        return std::make_shared<ApiImpl::MasterKeyImpl>(dataBlocks);
    }
    catch (...)
    {
        return nullptr;
    }
}

bool MasterKey::isValidBlock(const std::string &block)
{
    return Util::MasterKey::isPlausibleSingleBlock(block);
}

}
}
