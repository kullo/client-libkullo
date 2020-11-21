/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <cstdint>

namespace Kullo {

using id_type           = std::int64_t;
using lastModified_type = std::uint64_t;
using timestamp_type    = std::uint64_t;
using metaVersion_type  = std::uint32_t;

// maximum value a JSON number can hold
// 2^53, 52bits of mantissa + the implicit leading 1
const auto ID_MAX = id_type{1ULL<<53};
const auto ID_MIN = id_type{0};

}
