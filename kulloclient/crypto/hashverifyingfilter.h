/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <vector>

#include "kulloclient/util/filterchain.h"

namespace Kullo {
namespace Crypto {

class HashVerifyingFilter : public Util::Filter
{
public:
    HashVerifyingFilter(const std::vector<unsigned char> &expectedHash);
    ~HashVerifyingFilter() override;

    void write(
            Util::Sink &sink,
            const unsigned char *data,
            std::size_t length) override;
    void close(Util::Sink &sink) override;

private:
    void finalizeHash();

    const std::vector<unsigned char> expectedHash_;

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}
}
