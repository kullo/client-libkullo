/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <exception>

#include "kulloclient/api/LocalError.h"
#include "kulloclient/api/NetworkError.h"

namespace Kullo {
namespace ApiImpl {

Api::LocalError toLocalError(std::exception_ptr exptr);
Api::NetworkError toNetworkError(std::exception_ptr exptr);

}
}
