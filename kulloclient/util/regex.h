/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>
#include <vector>

#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Util {

// A wrapper helping identify regex exception sources
// See http://stackoverflow.com/a/30991243/2013738
//
// This adds the following value:
// - Make original pattern available in exception text
// - Add information of the function call causing the exception (e.g. construct/match()/replace())
//
// Uses the "Modified ECMAScript regular expression grammar" (http://en.cppreference.com/w/cpp/regex/ecmascript)
// to be compatible with STL implementations.
class Regex
{
public:
    Regex(const std::string &pattern);
    virtual ~Regex();
    std::string pattern() const;

    /**
     * @brief Determine if `search` matches the entire string `in`.
     */
    static bool match(
            const std::string &in,
            const Regex &search);

    /**
     * @brief Determine if `search` matches the entire string `in` and store submatches in `matches`.
     *
     * This function behaves like boost::regex_match(std::string, boost::smatch, boost::regex)
     * or std::regex_match(std::string, std::smatch, std::regex) except for the following
     * additional property:
     * If the function returns false, matches will be empty.
     */
    static bool match(
            const std::string &in,
            std::vector<std::string> &matches,
            const Regex &search);

    /**
     * @brief Search the first occurrence of `search` in `in`.
     */
    static bool search(
            const std::string &in,
            const Regex &search);

    /**
     * @brief Search the first occurrence of `search` in `in` and store submatches in `matches`.
     *
     * This function behaves like boost::regex_search(std::string, boost::smatch, boost::regex)
     * or std::regex_search(std::string, std::smatch, std::regex) except for the following
     * additional property:
     * If the function returns false, matches will be empty.
     */
    static bool search(
            const std::string &in,
            std::vector<std::string> &matches,
            const Regex &search);

    static std::string replace(
            const std::string &in,
            const Regex &search,
            const std::string &replace);

private:
    const std::string pattern_;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

/// An exception for errors while building or using regular expressions.
class RegexError : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    RegexError(const std::string &pattern, const std::string &message = "") throw()
        : BaseException("Pattern: " + pattern + "; message: " + message) {}
    virtual ~RegexError() = default;
};

}
}
