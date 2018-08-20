/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/hashverifyingfilter.h"

#include <botan/botan_all.h>
#include "kulloclient/crypto/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Crypto {

namespace {

auto HASH_FUNCTION = "SHA-512";

}

struct HashVerifyingFilter::Impl
{
    Impl()
        : hashFun(Botan::HashFunction::create(HASH_FUNCTION))
    {
        kulloAssert(hashFun);
    }

    std::unique_ptr<Botan::HashFunction> hashFun;
};

HashVerifyingFilter::HashVerifyingFilter(
        const std::vector<unsigned char> &expectedHash)
    : expectedHash_(expectedHash)
    , impl_(make_unique<Impl>())
{
    kulloAssertMsg(expectedHash_.size() == impl_->hashFun->output_length(),
                   "Wrong expected hash size: " + std::to_string(expectedHash_.size()));
}

HashVerifyingFilter::~HashVerifyingFilter()
{}

void HashVerifyingFilter::write(
        Util::Sink &sink, const unsigned char *data, std::size_t length)
{
    impl_->hashFun->update(data, length);
    sink.write(data, length);
}

void HashVerifyingFilter::close(Util::Sink &sink)
{
    K_UNUSED(sink);
    finalizeHash();
}

void HashVerifyingFilter::finalizeHash()
{
    std::vector<unsigned char> hash(impl_->hashFun->output_length());
    impl_->hashFun->final(hash.data());

    if (hash != expectedHash_)
    {
        throw HashDoesntMatch();
    }
}

}
}
