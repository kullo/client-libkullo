/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/masterkeyimpl.h"

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

MasterKeyImpl::MasterKeyImpl(const std::string &pem)
    : masterKey_(pem)
{
}

MasterKeyImpl::MasterKeyImpl(const std::vector<std::string> &dataBlocks)
    : masterKey_(dataBlocks)
{
}

MasterKeyImpl::MasterKeyImpl(const std::vector<unsigned char> &key)
    : masterKey_(key)
{
}

std::string MasterKeyImpl::pem()
{
    return masterKey_.toPem();
}

std::vector<std::string> MasterKeyImpl::dataBlocks()
{
    return masterKey_.dataBlocks();
}

bool MasterKeyImpl::isEqualTo(const std::shared_ptr<Api::MasterKey> &other)
{
    auto otherMasterKeyImpl = std::dynamic_pointer_cast<MasterKeyImpl>(other);
    kulloAssert(otherMasterKeyImpl);
    return masterKey_ == otherMasterKeyImpl->masterKey_;
}

}
}
