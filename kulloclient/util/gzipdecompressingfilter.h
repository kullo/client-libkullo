/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
