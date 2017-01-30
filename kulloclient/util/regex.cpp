/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/regex.h"

#if defined(_WIN32) // also defined on 64 bit
    #include <boost/regex.hpp>
    #define USE_BOOST_REGEX_IMPL
#else
    #include <regex>
#endif

#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Util {

// Use namespace instead of class to bridge types and methods in the same way
namespace Symbols {
#if defined(USE_BOOST_REGEX_IMPL)
    using boost::regex;
    using boost::regex_error;
    using boost::regex_match;
    using boost::regex_search;
    using boost::regex_replace;
    using boost::smatch;
#else
    using std::regex;
    using std::regex_error;
    using std::regex_match;
    using std::regex_search;
    using std::regex_replace;
    using std::smatch;
#endif
}

class Regex::Impl final
{
public:
    explicit Impl(const std::string &pattern)
        : regex(pattern)
    {
    }

    Symbols::regex regex;
};

Regex::Regex(const std::string &pattern)
    : pattern_(pattern)
{
    FWD_NESTED(
        impl_ = Kullo::make_unique<Regex::Impl>(pattern_),
        Symbols::regex_error,
        RegexError(pattern_, "Regex error in constructor")
    );
}

Regex::~Regex()
{
}

std::string Regex::pattern() const
{
    return pattern_;
}

bool Regex::match(const std::string &in, const Regex &search)
{
    FWD_NESTED(
        return Symbols::regex_match(in, search.impl_->regex),
        Symbols::regex_error,
        RegexError(search.pattern(), "Regex error in match()")
    );
}

bool Regex::match(const std::string &in, std::vector<std::string> &matches, const Regex &search)
{
    bool didMatch;
    Symbols::smatch internalMatches;

    matches.clear();

    FWD_NESTED(
        // Note: in Boost "if the function returns false, then the effect on parameter m is undefined"
        // http://www.boost.org/doc/libs/1_63_0/libs/regex/doc/html/boost_regex/ref/regex_match.html
        didMatch = Symbols::regex_match(in, internalMatches, search.impl_->regex),
        Symbols::regex_error,
        RegexError(search.pattern(), "Regex error in match()")
    );

    if (didMatch)
    {
        for (const auto &subMatch : internalMatches)
        {
            // type ssub_match http://en.cppreference.com/w/cpp/regex/sub_match
            matches.push_back(subMatch.str());
        }
    }

    return didMatch;
}

bool Regex::search(const std::string &in, const Regex &search)
{
    FWD_NESTED(
        return Symbols::regex_search(in, search.impl_->regex),
        Symbols::regex_error,
        RegexError(search.pattern(), "Regex error in search()")
    );
}

bool Regex::search(const std::string &in, std::vector<std::string> &matches, const Regex &search)
{
    bool didFind;
    Symbols::smatch internalMatches;

    matches.clear();

    FWD_NESTED(
        // Note: in Boost "if the function returns false, then the effect on parameter m is undefined"
        // http://www.boost.org/doc/libs/1_63_0/libs/regex/doc/html/boost_regex/ref/regex_search.html
        didFind = Symbols::regex_search(in, internalMatches, search.impl_->regex),
        Symbols::regex_error,
        RegexError(search.pattern(), "Regex error in search()")
    );

    if (didFind)
    {
        for (const auto &subMatch : internalMatches)
        {
            // type ssub_match http://en.cppreference.com/w/cpp/regex/sub_match
            matches.push_back(subMatch.str());
        }
    }

    return didFind;
}

std::string Regex::replace(
        const std::string &in,
        const Regex &search,
        const std::string &replace)
{
    FWD_NESTED(
        return Symbols::regex_replace(in, search.impl_->regex, replace),
        Symbols::regex_error,
        RegexError(search.pattern(), "Regex error in replace()")
    );
}

}
}
