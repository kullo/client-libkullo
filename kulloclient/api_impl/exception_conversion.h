/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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
