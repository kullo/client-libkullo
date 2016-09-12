/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/formatstring.h"

#include <iomanip>

#include <boost/regex.hpp>

namespace Kullo {
namespace Util {

namespace {
// Some chars must be escaped: .[{}()\*+?|^$
// See http://www.boost.org/doc/libs/1_56_0/libs/regex/doc/html/boost_regex/syntax/perl_syntax.html
// For the URI spec, see https://tools.ietf.org/html/rfc3986

// [unreserved, sub-delims, :, @] | pct-encoded
// Addition, not standardized: "[" and "]", which are illegal but commonly used
// & is replaced by &amp; because these regexes describe HTML-escaped URIs
const std::string URI_PCHAR = R"(([0-9a-zA-Z\-._~!\$'\(\)\*\+\,;=:@\[\]]|&amp;|(%[0-9a-fA-F]{2})))";

// (/pchar*)*
const std::string URI_PATH_ABEMPTY = std::string("(/") + URI_PCHAR + "*)*";

// (pchar | [/, ?])*
const std::string URI_QUERY = std::string("(") + URI_PCHAR + R"(|[/\?])*)";
const std::string URI_FRAGMENT = URI_QUERY;

const std::string HTTP_URI = std::string(
            "https?://"                         // scheme
            "[-a-zA-Z0-9]+(\\.[-a-zA-Z0-9]+)*"  // host
            "(:[0-9]+)?"                        // port (optional)
            + URI_PATH_ABEMPTY +                // path (optional)
            "(\\?" + URI_QUERY + ")?"           // query (optional)
            "(#" + URI_FRAGMENT + ")?"          // fragment (optional)
            );

const boost::regex LINKS_IN_MESSAGE_REGEX(
        std::string("(^|[\\t\\n\\r \\(])(") + HTTP_URI + ")"
        );

const boost::regex LINKS_IN_BRACKETS_REGEX(
        std::string("\\((") + HTTP_URI + ")\\)"
        );
}

std::string FormatString::padLeft(const std::string &input, const size_t targetLength, const char paddingChar)
{
    if(targetLength <= input.size()) return input;

    std::string out = input;
    out.insert(out.begin(), targetLength-out.size(), paddingChar);
    return out;
}

std::string FormatString::padRight(const std::string &input, const size_t targetLength, const char paddingChar)
{
    if(targetLength <= input.size()) return input;

    std::string out = input;
    out.insert(out.end(), targetLength-out.size(), paddingChar);
    return out;
}

std::string FormatString::toLower(const std::string &s)
{
    std::string out;
    std::transform(s.cbegin(), s.cend(), std::back_inserter(out), ::tolower);
    return out;
}

void FormatString::trim(std::string &s)
{
    // trim left
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    // trim right
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

void FormatString::highlightLinks(std::string &target)
{
    std::string buffer;
    buffer = boost::regex_replace(target,
                                  LINKS_IN_BRACKETS_REGEX,
                                  "(<a href=\"\\1\">\\1</a>)");
    buffer = boost::regex_replace(buffer,
                                  LINKS_IN_MESSAGE_REGEX,
                                  "\\1<a href=\"\\2\">\\2</a>");
    target.swap(buffer);
}

void FormatString::escapeMessageText(std::string &target)
{
    std::string buffer;
    buffer.reserve(std::string::size_type(std::round(target.size() * 1.10))); // prepare for 10 % overhead
    for(char &c : target)
    {
        switch(c)
        {
        case '&':  buffer.append("&amp;");  break;
        case '\"': buffer.append("&quot;"); break;
        case '<':  buffer.append("&lt;");   break;
        case '>':  buffer.append("&gt;");   break;
        default:   buffer.append(1, c);     break;
        }
    }
    target.swap(buffer);
}

void FormatString::messageTextToHtml(std::string &target)
{
    escapeMessageText(target);
    highlightLinks(target);
}

std::string FormatString::formatIntegerWithCommas(int value)
{
    auto is_negative = value < 0;
    std::string numberString = std::to_string(value);

    std::string out;

    std::vector<char> digits;
    if (is_negative)
    {
        digits = std::vector<char>(numberString.cbegin()+1, numberString.cend());
    }
    else
    {
        digits = std::vector<char>(numberString.cbegin(), numberString.cend());
    }

    while (digits.size() > 3)
    {
        out = "," + std::string(digits.end()-3, digits.end()) + out;
        digits.erase(digits.end()-3, digits.end());
    }

    auto digitsRemaining = std::string(digits.cbegin(), digits.cend());
    out = digitsRemaining + out;
    if (is_negative) out = "-" + out;

    return out;
}

}
}
