/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace Kullo {
namespace Crypto {

/// Provides cryptographical hash functions
class Hasher
{
public:
    static std::vector<unsigned char> sha256(const std::vector<unsigned char> &data);
    static std::string sha256Hex(const std::vector<unsigned char> &data);

    /**
     * @brief The SHA-512 hash function from the SHA-2 family
     * @param data The binary data to be hashed
     * @return The hash in binary form
     */
    static std::vector<unsigned char> sha512(const std::vector<unsigned char> &data);

    static std::string sha512Hex(const std::vector<unsigned char> &data);
    static std::string sha512Hex(std::istream &input);

    /// SHA256, shortened to 8 bytes. Not safe for cryptography!
    static std::int64_t eightByteHash(const std::vector<unsigned char> &data);
};

class HasherImpl;

class Sha512Hasher
{
public:
    Sha512Hasher();
    ~Sha512Hasher();
    void update(const std::vector<unsigned char> &data);
    void update(const unsigned char *data, std::size_t length);
    std::string hexDigest();

private:
    std::unique_ptr<HasherImpl> impl_;
};

}
}
