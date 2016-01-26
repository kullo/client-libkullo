/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

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
}
