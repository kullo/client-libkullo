/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Db {

/**
 * @brief An exception for the case that the integrity of the data
 *        that is stored in the local database is broken.
 *
 * A possible remedy is to delete the local database and re-sync.
 */
class DatabaseIntegrityError : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    DatabaseIntegrityError(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~DatabaseIntegrityError() = default;
};

}
}
