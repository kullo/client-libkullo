/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <sstream>
#include <string>

namespace Kullo {
namespace Util {

class Strings
{
public:
    static std::string padLeft(const std::string &input, const size_t targetLength, const char paddingChar = ' ');
    static std::string padRight(const std::string &input, const size_t targetLength, const char paddingChar = ' ');
    static std::string toLower(const std::string &s);
    static std::string trim(const std::string &input);
    /// Highlights web links in HTML-encoded text
    static std::string highlightWebLinks(const std::string &htmlIn);
    /// Highlights Kullo Adresses in HTML-encoded text. Skips anything that is part of a <a> tag
    static std::string highlightKulloAdresses(const std::string &htmlIn);
    /// Escapes HTML special chars
    static std::string htmlEscape(const std::string &plaintext);
    static std::string messageTextToHtml(const std::string &in, bool includeKulloAddresses = true);
    /// Formats integers to be easier on the human eye
    static std::string formatReadable(std::int64_t value);
};

template<typename InputIt>
std::string join(InputIt first, InputIt last, const std::string &separator)
{
    std::ostringstream result;
    for (auto iter = first; iter != last; ++iter)
    {
        if (iter != first) result << separator;
        result << *iter;
    }
    return result.str();
}

}
}
