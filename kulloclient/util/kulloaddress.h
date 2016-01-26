/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>

namespace Kullo {
namespace Util {

/// A Kullo address
class KulloAddress
{
public:
    /**
     * @brief Creates a new KulloAddress from an address string.
     *
     * Have a look at the Kullo protocol specification for details.
     */
    explicit KulloAddress(const std::string &address);

    /// Check Kullo address syntax
    static bool isValid(const std::string &address);

    /// Returns the full address
    std::string toString() const;

    /// Returns the full address in URL encoded form, e.g. "john.doe%23provider.net"
    std::string toUrlencodedString() const;

    /// Returns the username (e.g. "john.doe" for "john.doe#provider.net")
    std::string username() const;

    /// Returns the domain name (e.g. "kullo.net" for "john.doe#provider.net")
    std::string domain() const;

    /// Returns the server name (e.g. "kullo.provider.net" for "john.doe#provider.net")
    std::string server() const;

    bool operator==(const KulloAddress &other) const;
    bool operator!=(const KulloAddress &other) const;
    bool operator<(const KulloAddress &other) const;

    friend std::ostream &operator<<(std::ostream &out, const KulloAddress &address);

private:
    std::string username_, domain_;
};

}
}
