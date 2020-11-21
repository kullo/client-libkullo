/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/crypto/hasher.h"

#include <cerrno>
#include <cstring>
#include <botan/botan_all.h>

#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/hex.h"
#include "kulloclient/util/numeric_cast.h"

namespace Kullo {
namespace Crypto {

std::vector<unsigned char> Hasher::sha256(const std::vector<unsigned char> &data)
{
    return unlock(Botan::HashFunction::create("SHA-256")->process(data));
}

std::string Hasher::sha256Hex(const std::vector<unsigned char> &data)
{
    return Util::Hex::encode(sha256(data));
}

std::vector<unsigned char> Hasher::sha512(const std::vector<unsigned char> &data)
{
    return unlock(Botan::HashFunction::create("SHA-512")->process(data));
}

std::string Hasher::sha512Hex(const std::vector<unsigned char> &data)
{
    return Util::Hex::encode(sha512(data));
}

std::string Hasher::sha512Hex(std::istream &input)
{
    Sha512Hasher hasher;

    std::vector<unsigned char> readBuf(4096);
    std::size_t bytesRead = 0;
    do {
        input.read(reinterpret_cast<char *>(readBuf.data()), readBuf.size());
        bytesRead = Util::numeric_cast<std::size_t>(input.gcount());
        hasher.update(readBuf.data(), bytesRead);
    } while (input);

    if (!input.eof() || input.bad())
    {
        auto error = std::strerror(errno);
        throw Util::FilesystemError(
                    std::string("Error while calculating hash for stream")
                    + error);
    }

    return hasher.hexDigest();
}

std::int64_t Hasher::eightByteHash(const std::vector<unsigned char> &data)
{
    auto hash = sha256(data);
    std::int64_t result = 0;
    // big endian
    result |= (std::int64_t{hash[0]} << 56);
    result |= (std::int64_t{hash[1]} << 48);
    result |= (std::int64_t{hash[2]} << 40);
    result |= (std::int64_t{hash[3]} << 32);
    result |= (std::int64_t{hash[4]} << 24);
    result |= (std::int64_t{hash[5]} << 16);
    result |= (std::int64_t{hash[6]} <<  8);
    result |= (std::int64_t{hash[7]} <<  0);
    return result;
}

class HasherImpl
{
public:
    HasherImpl(const std::string &algorithm)
        : botanHasher_(Botan::HashFunction::create(algorithm))
    {
    }

    void update(const unsigned char *data, std::size_t length)
    {
        if (length > 0) botanHasher_->update(data, length);
    }

    std::string hexDigest()
    {
        return Util::Hex::encode(Botan::unlock(botanHasher_->final()));
    }

private:
    std::unique_ptr<Botan::HashFunction> botanHasher_;
};

Sha512Hasher::Sha512Hasher()
    : impl_(make_unique<HasherImpl>("SHA-512"))
{
}

Sha512Hasher::~Sha512Hasher()
{
}

void Sha512Hasher::update(const std::vector<unsigned char> &data)
{
    impl_->update(data.data(), data.size());
}

void Sha512Hasher::update(const unsigned char *data, std::size_t length)
{
    impl_->update(data, length);
}

std::string Sha512Hasher::hexDigest()
{
    return impl_->hexDigest();
}

}
}
