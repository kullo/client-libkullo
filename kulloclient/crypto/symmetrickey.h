/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace Kullo {
namespace Crypto {

struct SymmetricKeyImpl;

class SymmetricKey
{
public:
    /**
    * Create a new SymmetricKey
    * @param str is a hex encoded string
    */
    explicit SymmetricKey(const std::string& hexStr = "");

    explicit SymmetricKey(std::shared_ptr<SymmetricKeyImpl> priv);
    std::shared_ptr<SymmetricKeyImpl> priv() const;

    size_t bitSize() const;
    std::string toBase64() const;
    std::string toHex() const;
    std::vector<unsigned char> toVector() const;

    bool operator==(const SymmetricKey &other) const;
    friend std::ostream &operator<<(std::ostream &out,
                                    const SymmetricKey &symmKey);

private:
    std::shared_ptr<SymmetricKeyImpl> p;
};

}
}
