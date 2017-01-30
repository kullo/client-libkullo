/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/gzip.h"

#include <cstring>
#include <zlib.h>

#include "kulloclient/util/assert.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/librarylogger.h"

namespace Kullo {
namespace Util {

namespace {

const int LEVEL = 6; // default, much faster than 9 and almost as good results
const int WINDOW_SIZE = 15; // maximum
const int WINSIZE_OFFSET_GZIP = 16;
const int MEM_LEVEL = 8; // default

}

std::vector<unsigned char> GZip::compress(
        const std::vector<unsigned char> &data)
{
    ::z_stream zstream;
    std::memset(&zstream, 0, sizeof(zstream));
    zstream.next_in = const_cast<unsigned char *>(data.data());
    zstream.avail_in = data.size();

    auto rc = ::deflateInit2(
                &zstream, LEVEL, Z_DEFLATED, WINDOW_SIZE + WINSIZE_OFFSET_GZIP,
                MEM_LEVEL, Z_DEFAULT_STRATEGY);
    kulloAssert(rc == Z_OK);

    ::gz_header header;
    std::memset(&header, 0, sizeof(header));
    rc = ::deflateSetHeader(&zstream, &header);
    kulloAssert(rc == Z_OK);

    std::vector<unsigned char> compressed;
    compressed.resize(deflateBound(&zstream, data.size()));
    zstream.next_out = compressed.data();
    zstream.avail_out = compressed.size();

    rc = ::deflate(&zstream, Z_FINISH);
    if (rc != Z_STREAM_END)
    {
        throw GZipStreamError(
                    std::string("deflate error ") + std::to_string(rc) + ": "
                    + zstream.msg);
    }

    compressed.resize(zstream.total_out);

    rc = ::deflateEnd(&zstream);
    kulloAssert(rc == Z_OK);

    return compressed;
}

std::vector<unsigned char> GZip::decompress(
        const std::vector<unsigned char> &compressed)
{
    ::z_stream zstream;
    std::memset(&zstream, 0, sizeof(zstream));
    zstream.next_in = const_cast<unsigned char *>(compressed.data());
    zstream.avail_in = compressed.size();

    auto rc = ::inflateInit2(&zstream, WINDOW_SIZE + WINSIZE_OFFSET_GZIP);
    kulloAssert(rc == Z_OK);

    std::vector<unsigned char> decompressed(1 * 1024 * 1024, '\0');
    bool done = false;
    while (!done)
    {
        zstream.next_out = decompressed.data() + zstream.total_out;
        zstream.avail_out = decompressed.size() - zstream.total_out;
        rc = ::inflate(&zstream, Z_NO_FLUSH);
        switch (rc)
        {
        case Z_OK:
            continue;

        case Z_BUF_ERROR:
            if (zstream.avail_out > 0)
            {
                throw GZipStreamError("Z_BUF_ERROR but avail_out > 0");
            }

            // increase size heuristically to 150%
            decompressed.resize(decompressed.size() * 3 / 2);
            break;

        case Z_STREAM_END:
            // shrink to needed size
            decompressed.resize(zstream.total_out);
            done = true;
            break;

        default:
            throw GZipStreamError(
                        std::string("inflate error ") + std::to_string(rc)
                        + ": " + zstream.msg);
        }
    }

    rc = ::inflateEnd(&zstream);
    kulloAssert(rc == Z_OK);

    return decompressed;
}

}
}
