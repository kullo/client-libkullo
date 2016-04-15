/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/hasher.h"

#include <botan/botan.h>

#include "kulloclient/util/exceptions.h"

using namespace Botan;

namespace Kullo {
namespace Crypto {

std::vector<unsigned char> Hasher::sha256(const std::vector<unsigned char> &data)
{
    Pipe pipe(new Hash_Filter("SHA-256"));
    pipe.process_msg(data);
    return unlock(pipe.read_all(0));
}

std::string Hasher::sha256Hex(const std::vector<unsigned char> &data)
{
    Pipe pipe(new Hash_Filter("SHA-256"), new Hex_Encoder(Hex_Encoder::Lowercase));
    pipe.process_msg(data);
    auto result = pipe.read_all(0);
    return std::string(result.cbegin(), result.cend());
}

std::vector<unsigned char> Hasher::sha512(const std::vector<unsigned char> &data)
{
    Pipe pipe(new Hash_Filter("SHA-512"));
    pipe.process_msg(data);
    return unlock(pipe.read_all(0));
}

std::string Hasher::sha512Hex(const std::vector<unsigned char> &data)
{
    Pipe pipe(new Hash_Filter("SHA-512"), new Hex_Encoder(Hex_Encoder::Lowercase));
    pipe.process_msg(data);
    auto result = pipe.read_all(0);
    return std::string(result.cbegin(), result.cend());
}

std::string Hasher::sha512Hex(std::istream &input)
{
    try
    {
        Pipe pipe(new Hash_Filter("SHA-512"), new Hex_Encoder(Hex_Encoder::Lowercase));
        DataSource_Stream source(input);
        pipe.process_msg(source);
        auto result = pipe.read_all(0);
        return std::string(result.cbegin(), result.cend());
    }
    catch (Botan::Stream_IO_Error &)
    {
        std::throw_with_nested(Util::FilesystemError("Error while calculating hash for stream"));
    }
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

}
}
