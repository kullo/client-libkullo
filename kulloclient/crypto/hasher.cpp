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

}
}
