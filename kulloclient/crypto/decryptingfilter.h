/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <vector>

#include "kulloclient/crypto/symmetrickey.h"
#include "kulloclient/util/filterchain.h"

namespace Kullo {
namespace Crypto {

class DecryptingFilter : public Util::Filter
{
public:
    DecryptingFilter(const SymmetricKey &key);
    ~DecryptingFilter();

    void write(
            Util::Sink &sink,
            const unsigned char *data,
            std::size_t length) override;
    void close(Util::Sink &sink) override;

private:
    void doWrite(
            Util::Sink &sink,
            const unsigned char *data,
            std::size_t length);

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}
}
