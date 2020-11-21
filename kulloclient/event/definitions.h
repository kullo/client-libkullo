/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <unordered_set>

#include "kulloclient/api_impl/event.h"

namespace Kullo {
namespace Event {

using ApiEvents = std::unordered_set<Api::Event>;

}
}
