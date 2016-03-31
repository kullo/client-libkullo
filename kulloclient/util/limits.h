/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstddef>

namespace Kullo {
namespace Util {

const size_t kibibyte = 1024;
const size_t mebibyte = 1024 * kibibyte;

const size_t MESSAGE_KEY_SAFE_MAX_BYTES = 1 * kibibyte;
const size_t MESSAGE_CONTENT_MAX_BYTES = 64 * kibibyte;
const size_t MESSAGE_ATTACHMENTS_MAX_BYTES = 100 * mebibyte;
const size_t MESSAGE_ATTACHMENTS_SEND_MAX_BYTES = 100 * mebibyte;
const size_t MESSAGE_META_MAX_BYTES = 1 * kibibyte;

}
}
