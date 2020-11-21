/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
