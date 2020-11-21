/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
