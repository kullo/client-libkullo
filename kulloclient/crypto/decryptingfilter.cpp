/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/decryptingfilter.h"

#include <algorithm>
#include <limits>

#include <botan/botan.h>
#include "kulloclient/crypto/symmetrickeyimpl.h"
#include "kulloclient/crypto/symmetriccryptor.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Crypto {

namespace {

const std::string CIPHER = "AES-256/GCM";

}

struct DecryptingFilter::Impl
{
    Impl(const SymmetricKey &key)
        : cipher_(Botan::get_cipher_mode(CIPHER, Botan::Cipher_Dir::DECRYPTION))
        , iv_(Util::to_vector(SymmetricCryptor::ATTACHMENTS_IV))
    {
        kulloAssert(cipher_);
        cipher_->set_key(key.priv()->key);
    }

    std::unique_ptr<Botan::Cipher_Mode> cipher_;
    const std::vector<unsigned char> iv_;
    bool writtenTo_ = false;
    std::vector<unsigned char> buffer_;
};

DecryptingFilter::DecryptingFilter(const SymmetricKey &key)
    : impl_(make_unique<Impl>(key))
{}

DecryptingFilter::~DecryptingFilter()
{}

void DecryptingFilter::write(
        Util::Sink &sink, const unsigned char *data, std::size_t length)
{
    auto &buf = impl_->buffer_;
    auto tagSize = impl_->cipher_->tag_size();

    if (!impl_->writtenTo_)
    {
        auto out = impl_->cipher_->start(impl_->iv_.data(), impl_->iv_.size());
        sink.write(out.data(), out.size());
        impl_->writtenTo_ = true;
        buf.reserve(tagSize);
    }

    // preserve only (potential) tag, but limit it to data's length
    auto dataBytesToKeep = std::min(tagSize, length);
    auto dataBytesToWrite = length - dataBytesToKeep;

    // preserve what is necessary to complete the (potential) tag from data,
    // but don't exceed the buffer's length
    auto bufferBytesToKeep = std::min(tagSize - dataBytesToKeep, buf.size());
    auto bufferBytesToWrite = buf.size() - bufferBytesToKeep;

    // write as many data bytes as possible to sink
    doWrite(sink, buf.data(), bufferBytesToWrite);
    doWrite(sink, data, dataBytesToWrite);

    // new buffer contents = preserved from old buffer + preserved from data
    if (bufferBytesToWrite > 0)
    {
        // safely convert bufferBytesToWrite to the (signed) difference type
        using diff_t = std::vector<unsigned char>::iterator::difference_type;
        using unsigned_diff_t = std::make_unsigned<diff_t>::type;
        auto diffMax = std::numeric_limits<diff_t>::max();
        kulloAssert(bufferBytesToWrite <= static_cast<unsigned_diff_t>(diffMax));
        auto offset = static_cast<diff_t>(bufferBytesToWrite);

        // remove data that has been written to sink
        std::copy(buf.begin() + offset, buf.end(), buf.begin());
        buf.resize(bufferBytesToKeep);
    }
    std::copy(data + dataBytesToWrite, data + length, std::back_inserter(buf));
}

void DecryptingFilter::close(Util::Sink &sink)
{
    if (impl_->writtenTo_)
    {
        auto &buf = impl_->buffer_;
        Botan::secure_vector<unsigned char> output(buf.begin(), buf.end());
        impl_->cipher_->finish(output);
        sink.write(output.data(), output.size());
    }
}

void DecryptingFilter::doWrite(Util::Sink &sink, const unsigned char *data, std::size_t length)
{
    if (length > 0)
    {
        Botan::secure_vector<unsigned char> inout(data, data + length);
        impl_->cipher_->update(inout);
        sink.write(inout.data(), inout.size());
    }
}

}
}
