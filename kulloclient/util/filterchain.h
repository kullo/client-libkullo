/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace Kullo {
namespace Util {

class Sink
{
public:
    virtual ~Sink() = default;

    virtual void write(const unsigned char *buffer, std::size_t size) = 0;
    virtual void close() {}
};

class NullSink : public Util::Sink
{
public:
    void write(const unsigned char *buffer, std::size_t size) override;
};

class BackInsertingSink : public Util::Sink
{
public:
    BackInsertingSink(std::vector<unsigned char> &container);
    void write(const unsigned char *buffer, std::size_t size) override;

private:
    std::vector<unsigned char> &container_;
};


class Filter
{
public:
    virtual ~Filter() = default;

    virtual void write(
            Sink &sink, const unsigned char *buffer, std::size_t size) = 0;
    virtual void close(Sink &/*sink*/) {}
};

class NullFilter : public Util::Filter
{
public:
    void write(
            Util::Sink &sink,
            const unsigned char *buffer,
            std::size_t size) override;
};


class FilterChain final
{
public:
    FilterChain(std::unique_ptr<Sink> &&sink);
    ~FilterChain();

    /**
     * @brief Pushes a filter on the filter stack.
     *
     * The pushed filter becomes the first filter in the chain.
     */
    void pushFilter(std::unique_ptr<Filter> &&filter);

    /**
     * @brief Feeds the given data into the chain.
     *
     * Must not be called after close() has been called.
     *
     * @param buffer Allowed to be nullptr iff size == 0
     */
    void write(const unsigned char *buffer, std::size_t size);

    template<typename Alloc>
    void write(const std::vector<unsigned char, Alloc> &input)
    {
        write(input.data(), input.size());
    }

    /**
     * @brief Close all filters and the sink.
     *
     * This will be called automatically on destruction if it hasn't been called
     * before. However, it should be called earlier because the dtor will
     * suppress exceptions.
     *
     * No other methods of the chain should be called afterwards.
     */
    void close();

private:
    class FilterSink : public Sink
    {
    public:
        FilterSink(std::unique_ptr<Filter> &&filter, Sink &sink);
        void write(const unsigned char *buffer, std::size_t size) override;
        void close() override;

    private:
        std::unique_ptr<Filter> filter_;
        Sink &sink_;
    };

    std::unique_ptr<Sink> sink_;
    // filters_ must be placed after sink_ so that it is destructed first,
    // because if nonempty, filters_.front() contains a reference to sink_.
    // Additionally, we need to use unique_ptr<FilterSink> instead of FilterSink
    // because otherwise the references to the sink_ members would be
    // invalidated on each resizing of the vector.
    std::vector<std::unique_ptr<FilterSink>> filters_;
    bool dataWritten_ = false;
    bool closed_ = false;
};

}
}
