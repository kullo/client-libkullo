/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>
#include <ios>
#include <memory>
#include <vector>

namespace Kullo {
namespace Crypto {

struct ClosableMulticharOutputFilterTag
        : boost::iostreams::closable_tag
        , boost::iostreams:: multichar_output_filter_tag
{};

class HashVerifyingFilter final
{
public:
    using char_type = char;
    using category = ClosableMulticharOutputFilterTag;

    HashVerifyingFilter(const std::vector<unsigned char> &expectedHash);
    ~HashVerifyingFilter();

    template<typename Sink>
    std::streamsize write(Sink &sink, const char *data, std::streamsize length)
    {
        updateHash(data, length);
        return boost::iostreams::write(sink, data, length);
    }

    template<typename Device>
    void close(Device&)
    {
        finalizeHash();
    }

    std::vector<unsigned char> hash() const;

private:
    void updateHash(const char *data, std::streamsize length);
    void finalizeHash();

    const std::vector<unsigned char> expectedHash_;

    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::vector<unsigned char> hash_;
};

}
}
