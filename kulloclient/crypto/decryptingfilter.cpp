/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/decryptingfilter.h"

#include <algorithm>
#include <iterator>
#include <limits>

#include <botan/botan_all.h>
#include "kulloclient/crypto/exceptions.h"
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
    auto minFinalSize = impl_->cipher_->minimum_final_size();

    if (!impl_->writtenTo_)
    {
        impl_->cipher_->start(impl_->iv_.data(), impl_->iv_.size());
        impl_->writtenTo_ = true;
        buf.reserve(minFinalSize);
    }

    // all bytes that can be written right now (excluding bytes for final update)
    auto totalBytesAvailable = buf.size() + length > minFinalSize
            ? buf.size() + length - minFinalSize
            : size_t{0};

    // writing must take place in multiples of update_granularity bytes
    auto totalBytesToWrite =
            totalBytesAvailable
            - totalBytesAvailable % impl_->cipher_->update_granularity();

    auto bufferBytesToWrite = std::min(totalBytesToWrite, buf.size());
    auto bufferBytesToKeep = buf.size() - bufferBytesToWrite;

    auto dataBytesToWrite = totalBytesToWrite - bufferBytesToWrite;

    // copy data to write in one contiguous buffer
    std::vector<unsigned char> writeBuf;
    std::copy(buf.data(), buf.data() + bufferBytesToWrite, std::back_inserter(writeBuf));
    std::copy(data, data + dataBytesToWrite, std::back_inserter(writeBuf));
    kulloAssert(writeBuf.size() == totalBytesToWrite);

    // write data to sink
    doWrite(sink, writeBuf.data(), writeBuf.size());

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
        try
        {
            impl_->cipher_->finish(output);
        }
        catch (Botan::Integrity_Failure&)
        {
            std::throw_with_nested(
                        IntegrityFailure("DecryptingFilter::doWrite()"));
        }
        sink.write(output.data(), output.size());
    }
}

void DecryptingFilter::doWrite(Util::Sink &sink, const unsigned char *data, std::size_t length)
{
    if (length > 0)
    {
        Botan::secure_vector<unsigned char> inout(data, data + length);
        try
        {
            impl_->cipher_->update(inout);
        }
        catch (Botan::Integrity_Failure&)
        {
            std::throw_with_nested(
                        IntegrityFailure("DecryptingFilter::doWrite()"));
        }
        sink.write(inout.data(), inout.size());
    }
}

}
}
