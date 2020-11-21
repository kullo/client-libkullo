/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <cstddef>

namespace Kullo {
namespace Util {

const size_t kibibyte = 1024;
const size_t mebibyte = 1024 * kibibyte;

const size_t MESSAGE_KEY_SAFE_MAX_BYTES = 1 * kibibyte;
const size_t MESSAGE_CONTENT_MAX_BYTES = 64 * kibibyte;
const size_t MESSAGE_ATTACHMENTS_MAX_BYTES = 100 * mebibyte;
const size_t MESSAGE_META_MAX_BYTES = 1 * kibibyte;

}
}
