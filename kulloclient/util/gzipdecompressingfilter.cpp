/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/gzipdecompressingfilter.h"

#include <botan/botan_all.h>

#include "kulloclient/util/assert.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Util {

namespace {

const std::string ALGORITHM = "gzip";

}

struct GZipDecompressingFilter::Impl
{
    Impl()
        : decompressor_(Botan::make_decompressor(ALGORITHM))
    {
        kulloAssert(decompressor_);
        decompressor_->start();
    }

    std::unique_ptr<Botan::Decompression_Algorithm> decompressor_;
    bool bad_ = false;
    bool hasSeenInput_ = false;
};

GZipDecompressingFilter::GZipDecompressingFilter()
    : impl_(make_unique<Impl>())
{}

GZipDecompressingFilter::~GZipDecompressingFilter()
{}

void GZipDecompressingFilter::write(
        Sink &sink, const unsigned char *buffer, std::size_t size)
{
    Botan::secure_vector<unsigned char> inout(buffer, buffer + size);
    if (inout.size() > 0) impl_->hasSeenInput_ = true;

    // set bad_ flag so that an exception is not thrown both on write and close
    try {
        if (!inout.empty()) impl_->decompressor_->update(inout);
    } catch (...) {
        impl_->bad_ = true;
        throw;
    }
    if (!inout.empty()) sink.write(inout.data(), inout.size());
}

void GZipDecompressingFilter::close(Sink &sink)
{
    if (!impl_->bad_ && impl_->hasSeenInput_)
    {
        Botan::secure_vector<unsigned char> output;
        impl_->decompressor_->finish(output);
        if (!output.empty()) sink.write(output.data(), output.size());
    }
}

}
}
