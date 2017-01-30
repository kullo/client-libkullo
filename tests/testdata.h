/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>
#include <vector>

#include <kulloclient/util/binary.h>

namespace MasterKeyData {

const std::string INVALID_PEM {
    "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
    "Version: 1 (256 bit)\n"
    "\n"
    "100000 000000 000000 000000\n"
    "000000 000000 000000 000000\n"
    "000000 000000 000000 000000\n"
    "000000 000000 000000 000000\n"
    "-----END KULLO PRIVATE MASTER KEY-----\n"
};
const std::string VALID_PEM {
    "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
    "Version: 1 (256 bit)\n"
    "\n"
    "000000 000000 000000 000000\n"
    "000000 000000 000000 000000\n"
    "000000 000000 000000 000000\n"
    "000000 000000 000000 000000\n"
    "-----END KULLO PRIVATE MASTER KEY-----\n"
};
const std::vector<std::string> INVALID_DATA_BLOCKS {
    "100000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000"
};
const std::vector<std::string> VALID_DATA_BLOCKS {
    "000000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000"
};
const std::vector<std::string> VALID_DATA_BLOCKS2 {
    "009167", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000",
    "000000", "000000", "000000", "000000"
};

} // namespace MasterKeyData

namespace GZipData {

const std::vector<unsigned char> HELLO_WORLD =
        Kullo::Util::to_vector("Hello World\n");

// echo "Hello World" | gzip | hexdump -C
const std::vector<unsigned char> HELLO_WORLD_GZIPPED = {
        0x1f, 0x8b, 0x08, 0x00, 0xd6, 0x02, 0x87, 0x54,
        0x00, 0x03, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0x57,
        0x08, 0xcf, 0x2f, 0xca, 0x49, 0xe1, 0x02, 0x00,
        0xe3, 0xe5, 0x95, 0xb0, 0x0c, 0x00, 0x00, 0x00
};

// echo -n "" | gzip | hexdump -C
const std::vector<unsigned char> EMTPY_GZIPPED = {
        0x1f, 0x8b, 0x08, 0x00, 0xad, 0x06, 0x87, 0x54,
        0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
};

} // namespace GZipData
