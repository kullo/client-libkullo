/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "kulloclient/util/filterchain.h"

namespace Kullo {
namespace Codec {

class SizeLimitingFilter : public Util::Filter
{
public:
    SizeLimitingFilter(const std::size_t maxSize);

    void write(
            Util::Sink &sink,
            const unsigned char *data,
            std::size_t length) override;

private:
    const std::size_t maxSize_;
    std::size_t bytesWritten_ = 0;
};

}
}
