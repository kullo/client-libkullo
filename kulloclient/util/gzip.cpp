/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/gzip.h"

#include <botan/botan.h>

namespace Kullo {
namespace Util {

namespace {

const std::string ALGORITHM = "gzip";
const size_t LEVEL = 9;  // maximum

}

std::vector<unsigned char> GZip::compress(
        const std::vector<unsigned char> &data)
{
    Botan::secure_vector<unsigned char> compressed(data.begin(), data.end());

    std::unique_ptr<Botan::Compressor_Transform> compressor(
                Botan::make_compressor(ALGORITHM, LEVEL));
    compressor->start();
    compressor->finish(compressed);

    return Botan::unlock(compressed);
}

std::vector<unsigned char> GZip::decompress(
        const std::vector<unsigned char> &compressed)
{
    Botan::secure_vector<unsigned char> decompressed(
                compressed.begin(), compressed.end());

    std::unique_ptr<Botan::Compressor_Transform> decompressor(
                Botan::make_decompressor(ALGORITHM));
    decompressor->start();
    decompressor->finish(decompressed);

    return Botan::unlock(decompressed);
}

}
}
