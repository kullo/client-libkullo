/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Crypto {

class HashDoesntMatch : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    HashDoesntMatch(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~HashDoesntMatch() = default;
};

}
}
