/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Sync {

class SyncCanceled : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    SyncCanceled(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~SyncCanceled() = default;
};

}
}
