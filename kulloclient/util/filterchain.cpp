/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/filterchain.h"

#include <algorithm>

#include "kulloclient/util/assert.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Util {

void NullSink::write(const unsigned char *buffer, std::size_t size)
{
    K_UNUSED(buffer);
    K_UNUSED(size);
}

void NullFilter::write(Sink &sink, const unsigned char *buffer, std::size_t size)
{
    sink.write(buffer, size);
}

BackInsertingSink::BackInsertingSink(std::vector<unsigned char> &container)
    : container_(container)
{}

void BackInsertingSink::write(const unsigned char *buffer, std::size_t size)
{
    container_.reserve(container_.size() + size);

    auto end = buffer + size;
    while (buffer < end)
    {
        container_.push_back(*buffer);
        ++buffer;
    }
}

FilterChain::FilterChain(std::unique_ptr<Sink> &&sink)
    : sink_(std::move(sink))
{
    kulloAssert(sink_);
}

FilterChain::~FilterChain()
{
    try
    {
        if (!closed_) close();
    }
    // dtors must not throw
    catch (...) {}
}

void FilterChain::pushFilter(std::unique_ptr<Filter> &&filter)
{
    kulloAssert(filter);
    kulloAssert(!dataWritten_);
    kulloAssert(!closed_);

    auto &sink = (filters_.empty()) ? *sink_ : *filters_.back();
    filters_.emplace_back(make_unique<FilterSink>(std::move(filter), sink));
}

void FilterChain::write(const unsigned char *buffer, std::size_t size)
{
    kulloAssert(buffer || !size);
    kulloAssert(!closed_);

    dataWritten_ = true;
    if (filters_.empty())
    {
        sink_->write(buffer, size);
    }
    else
    {
        filters_.back()->write(buffer, size);
    }
}

void FilterChain::close()
{
    kulloAssert(!closed_);

    closed_ = true;

    std::for_each(
                filters_.rbegin(), filters_.rend(),
                [](std::unique_ptr<FilterSink> &filter)
    {
        filter->close();
    });
    sink_->close();
}

FilterChain::FilterSink::FilterSink(std::unique_ptr<Filter> &&filter, Sink &sink)
    : filter_(std::move(filter))
    , sink_(sink)
{
    kulloAssert(filter_);
}

void FilterChain::FilterSink::write(const unsigned char *buffer, std::size_t size)
{
    filter_->write(sink_, buffer, size);
}

void FilterChain::FilterSink::close()
{
    filter_->close(sink_);
}

}
}
