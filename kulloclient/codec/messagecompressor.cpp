/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/codec/messagecompressor.h"

#include "kulloclient/util/gzip.h"

namespace Kullo {
namespace Codec {

void MessageCompressor::compress(EncodedMessage &msg) const
{
    msg.content = Util::GZip::compress(msg.content);
    if (!msg.attachments.empty())
    {
        msg.attachments = Util::GZip::compress(msg.attachments);
    }
}

std::vector<unsigned char> MessageCompressor::decompress(const std::vector<unsigned char> &data) const
{
    return Util::GZip::decompress(data);
}

}
}
