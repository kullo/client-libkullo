/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Crypto {

class HashDoesntMatch : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    HashDoesntMatch(const std::string &message = "") throw() : BaseException(message) {}
    ~HashDoesntMatch() override = default;
};

class IntegrityFailure : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    IntegrityFailure(const std::string &message = "") throw() : BaseException(message) {}
    ~IntegrityFailure() override = default;
};

}
}
