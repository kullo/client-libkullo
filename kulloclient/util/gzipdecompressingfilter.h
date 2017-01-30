/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>

#include "kulloclient/util/filterchain.h"

namespace Kullo {
namespace Util {

class GZipDecompressingFilter : public Filter
{
public:
    GZipDecompressingFilter();
    ~GZipDecompressingFilter() override;

    void write(
            Sink &sink, const unsigned char *buffer, std::size_t size) override;
    void close(Sink &sink) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}
}
