/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>
#include <vector>

#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Crypto {

class PBKDFFailed : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    PBKDFFailed(const std::string &message = "") throw() : BaseException(message) {}
    ~PBKDFFailed() override = default;
};

class PBKDF
{
public:
    static std::vector<unsigned char> argon2i(
            uint32_t times, uint32_t memoryKiB, uint32_t parallelism,
            const std::string &password,
            const std::vector<unsigned char> &salt,
            size_t outputBytes);
};

}
}
