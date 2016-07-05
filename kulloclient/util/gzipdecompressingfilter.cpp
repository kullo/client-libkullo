/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/gzipdecompressingfilter.h"

#include <botan/botan_all.h>
#include <zlib.h>

#include "kulloclient/util/assert.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Util {

namespace {

const std::string ALGORITHM = "gzip";
const int WINDOW_SIZE = 15; // maximum
const int WINSIZE_OFFSET_GZIP = 16;

}

struct GZipDecompressingFilter::Impl
{
    Impl()
        : outputBuffer_(1 * 1024 * 1024, '\0')
    {
        std::memset(&zstream_, 0, sizeof(zstream_));
        auto rc = ::inflateInit2(&zstream_, WINDOW_SIZE + WINSIZE_OFFSET_GZIP);
        kulloAssert(rc == Z_OK);
    }

    ~Impl()
    {
        if (::inflateEnd(&zstream_) != Z_OK)
        {
            // don't throw from dtor
            Log.e() << "Error during inflateEnd: " << zstream_.msg;
        }
    }

    ::z_stream zstream_;
    bool hasSeenInput_ = false;
    bool finished_ = false;
    std::vector<unsigned char> outputBuffer_;
};

GZipDecompressingFilter::GZipDecompressingFilter()
    : impl_(make_unique<Impl>())
{}

GZipDecompressingFilter::~GZipDecompressingFilter()
{}

void GZipDecompressingFilter::write(
        Sink &sink, const unsigned char *buffer, std::size_t size)
{
    if (!size) return;
    impl_->hasSeenInput_ = true;

    if(impl_->finished_)
    {
        throw GZipStreamError("Tried to write to a finished stream");
    }

    impl_->zstream_.next_in = const_cast<unsigned char *>(buffer);
    impl_->zstream_.avail_in = size;

    while (!impl_->finished_)
    {
        impl_->zstream_.next_out = impl_->outputBuffer_.data();
        impl_->zstream_.avail_out = impl_->outputBuffer_.size();

        auto rc = ::inflate(&impl_->zstream_, Z_NO_FLUSH);
        switch (rc)
        {
        case Z_OK:
            break;

        case Z_STREAM_END:
            impl_->finished_ = true;
            break;

        default:
            impl_->finished_ = true;

            throw GZipStreamError(
                        std::string("inflate error ") + std::to_string(rc)
                        + ": " + impl_->zstream_.msg);
        }

        auto bytesProduced = impl_->outputBuffer_.size() - impl_->zstream_.avail_out;
        if (bytesProduced)
        {
            sink.write(impl_->outputBuffer_.data(), bytesProduced);
        }

        // stream is finished, but there's some input left
        if (impl_->finished_ && (impl_->zstream_.avail_in > 0))
        {
            throw GZipStreamError("Encountered extra input after stream end");
        }

        // we're done if the whole input buffer has been processed
        if (!impl_->zstream_.avail_in) break;
    }
}

void GZipDecompressingFilter::close(Sink &sink)
{
    K_UNUSED(sink);

    if (impl_->hasSeenInput_ && !impl_->finished_)
    {
        throw GZipStreamError("Tried to close non-empty but unfinished stream");
    }
}

}
}
