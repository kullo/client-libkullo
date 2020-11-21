/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/codec/sizelimitingfilter.h"

#include "kulloclient/codec/exceptions.h"

namespace Kullo {
namespace Codec {

SizeLimitingFilter::SizeLimitingFilter(const std::size_t maxSize)
    : maxSize_(maxSize)
{}

void SizeLimitingFilter::write(
        Util::Sink &sink, const unsigned char *data, std::size_t length)
{
    bytesWritten_ += length;
    if (bytesWritten_ > maxSize_)
    {
        throw InvalidContentFormat("Attachments are too large");
    }
    sink.write(data, length);
}

}
}
