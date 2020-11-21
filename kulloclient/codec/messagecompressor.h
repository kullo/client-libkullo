/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <vector>

#include "kulloclient/codec/codecstructs.h"

namespace Kullo {
namespace Codec {

class MessageCompressor
{
public:
    virtual ~MessageCompressor() = default;

    virtual void compress(EncodedMessage &msg) const;
    virtual std::vector<unsigned char> decompress(const std::vector<unsigned char> &data) const;
};

}
}
