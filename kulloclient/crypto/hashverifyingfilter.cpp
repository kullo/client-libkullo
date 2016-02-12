/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/hashverifyingfilter.h"

#include <botan/botan.h>
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
{}

HashVerifyingFilter::~HashVerifyingFilter()
{}

std::vector<unsigned char> HashVerifyingFilter::hash() const
{
    return hash_;
}

void HashVerifyingFilter::updateHash(const char *data, std::streamsize length)
{
    kulloAssert(length >= 0);
    impl_->hashFun->update(
                reinterpret_cast<const unsigned char*>(data),
                static_cast<size_t>(length));
}

void HashVerifyingFilter::finalizeHash()
{
    hash_.resize(impl_->hashFun->output_length());
    impl_->hashFun->final(hash_.data());

    if (hash_ != expectedHash_)
    {
        throw HashDoesntMatch();
    }
}

}
}
