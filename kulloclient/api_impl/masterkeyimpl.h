/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/MasterKey.h"
#include "kulloclient/util/masterkey.h"

namespace Kullo {
namespace ApiImpl {

class MasterKeyImpl : public Api::MasterKey
{
public:
    explicit MasterKeyImpl(const std::string &pem);
    explicit MasterKeyImpl(const std::vector<std::string> &dataBlocks);
    explicit MasterKeyImpl(const std::vector<unsigned char> &key);

    std::string pem() override;
    std::vector<std::string> dataBlocks() override;
    bool isEqualTo(const std::shared_ptr<Api::MasterKey> &other) override;

private:
    Util::MasterKey masterKey_;
};

}
}
