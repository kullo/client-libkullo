/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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
