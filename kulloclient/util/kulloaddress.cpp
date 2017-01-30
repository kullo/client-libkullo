/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/kulloaddress.h"

#include <algorithm>
#include <iterator>
#include <ostream>

#include "kulloclient/util/misc.h"
#include "kulloclient/util/regex.h"

namespace Kullo {
namespace Util {

namespace {
// ECMAScript (the default)
// See http://en.cppreference.com/w/cpp/regex/ecmascript
//
// check domain name
// alphanum with embedded minus: [a-z0-9]+(?:-[a-z0-9]+)*
const Regex DOMAIN_REGEX(
                    "(?:"
                        "[a-z0-9]+(?:-[a-z0-9]+)*"
                    "\\.)+"
                    "[a-z][a-z0-9]*(?:-[a-z0-9]+)*"
            );
const int DOMAIN_MAX_LENGTH = 255;

// check username
// X with embedded separators: X([.-_]X)*
// X is alphanumeric and lowercase.
const Regex USERNAME_REGEX(
                    "[a-z0-9]+"                      // username part
                    "(?:"
                        "[-\\._]"                    // separator: -._
                        "[a-z0-9]+"                  // username part
                    ")*"
            );
const int USERNAME_MAX_LENGTH = 64;
}

KulloAddress::KulloAddress(const std::string &address)
{
    if (std::count(address.cbegin(), address.cend(), '#') != 1)
    {
        throw std::invalid_argument("KulloAddress(): address must contain exactly one '#'");
    }

    // copy and convert to lowercase
    auto hashPos = std::find(address.cbegin(), address.cend(), '#');
    std::transform(address.cbegin(), hashPos, std::back_inserter(username_), ::tolower);
    std::transform(hashPos + 1, address.cend(), std::back_inserter(domain_), ::tolower);

    if (username_.length() > USERNAME_MAX_LENGTH || !Regex::match(username_, USERNAME_REGEX))
    {
        throw std::invalid_argument("KulloAddress(): invalid username");
    }

    if (domain_.length() > DOMAIN_MAX_LENGTH || !Regex::match(domain_, DOMAIN_REGEX))
    {
        throw std::invalid_argument("KulloAddress(): invalid domain name");
    }

    auto lastLabelBoundary = domain_.cbegin();
    auto currentLabelBoundary = domain_.cbegin();
    while (currentLabelBoundary != domain_.cend())
    {
        lastLabelBoundary = currentLabelBoundary;
        ++currentLabelBoundary;
        currentLabelBoundary = std::find(currentLabelBoundary, domain_.cend(), '.');
        if (std::distance(lastLabelBoundary, currentLabelBoundary) - 1 > 63) {
            throw std::invalid_argument("KulloAddress(): domain label too long");
        }
    }
}

bool KulloAddress::isValid(const std::string &address)
{
    try
    {
        KulloAddress{address};
    }
    catch(std::invalid_argument)
    {
        return false;
    }
    return true;
}

std::string KulloAddress::toString() const
{
    return username_ + "#" + domain_;
}

std::string KulloAddress::toUrlencodedString() const
{
    return username_ + "%23" + domain_;
}

std::string KulloAddress::username() const
{
    return username_;
}

std::string KulloAddress::domain() const
{
    return domain_;
}

std::string KulloAddress::server() const
{
    return "kullo." + domain_;
}

bool KulloAddress::operator==(const KulloAddress &other) const
{
    return (username_ == other.username_) && (domain_ == other.domain_);
}

bool KulloAddress::operator!=(const KulloAddress &other) const
{
    return !(*this == other);
}

bool KulloAddress::operator<(const KulloAddress &other) const
{
    return toString() < other.toString();
}

std::ostream &operator<<(std::ostream &out, const KulloAddress &address)
{
    out << address.toString();
    return out;
}

}
}
