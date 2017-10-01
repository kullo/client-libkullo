/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/strings.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <functional>
#include <iomanip>
#include <iterator>
#include <vector>

#include "kulloclient/util/regex.h"

namespace Kullo {
namespace Util {

namespace {
// Some chars must be escaped: ^ $ \ . * + ? ( ) [ ] { } |
// See http://www.boost.org/doc/libs/1_62_0/libs/regex/doc/html/boost_regex/syntax/perl_syntax.html
// and http://ecma-international.org/ecma-262/5.1/#sec-15.10.1
// For the URI spec, see https://tools.ietf.org/html/rfc3986
//
// Use non-marking subexpressions (?:xyz) instead of marked subexpressions (xyz)
// for the HTTP URI part in order to avoid increasing the backreference count.
// ECMAScript does not support named backreferences.

// [unreserved, sub-delims, :, @] | pct-encoded
// & is replaced by &amp; because these regexes describe HTML-escaped URIs
std::string makeUriPcharPattern(const std::string &specialChars)
{
    return
            "(?:"
                "&amp;"
                "|"
                "(?:"
                    "%[0-9a-fA-F]{2}"
                ")"
                "|"
                R"([-0-9a-zA-Z\._~!\$'\(\)\*\+,;=:@)" + specialChars + "]"
            ")";
}

// Addition, not standardized: "[" and "]", which are illegal but commonly used
// pchar | [/, ?]
const std::string URI_PCHAR_PLUS = makeUriPcharPattern("\\[\\]/\\?");

const std::string URI_PCHAR_PATH = makeUriPcharPattern(
            "\\[\\]"
            // Allowed for the .de TLD (https://de.wikipedia.org/wiki/Internationalisierter_Domainname#Zeichens.C3.A4tze)
            "àáâãäåæāăąçćĉċčďđèéêëēĕėęěĝğġģĥħìíîïĩīĭįıðĵķĸĺļľłñńņňŋòóôõöøōŏőœŕŗřśŝşšţťŧùúûüũūŭůűųÿŷŵýźżžþß"
            // Upper case: python3 -c "print('àáâãäåæ...'.upper())"
            "ÀÁÂÃÄÅÆĀĂĄÇĆĈĊČĎĐÈÉÊËĒĔĖĘĚĜĞĠĢĤĦÌÍÎÏĨĪĬĮIÐĴĶĸĹĻĽŁÑŃŅŇŊÒÓÔÕÖØŌŎŐŒŔŖŘŚŜŞŠŢŤŦÙÚÛÜŨŪŬŮŰŲŸŶŴÝŹŻŽÞ"
            );

// (/pchar*)*
const std::string URI_PATH_ABEMPTY = "(?:/" + URI_PCHAR_PATH + "*)*";

// (pchar | [/, ?])*
const std::string URI_QUERY = URI_PCHAR_PLUS + "*";
const std::string URI_FRAGMENT = URI_QUERY;

const std::string DOMAIN_CHARS =
        "-a-zA-Z0-9"
        // Allowed for the .de TLD (https://de.wikipedia.org/wiki/Internationalisierter_Domainname#Zeichens.C3.A4tze)
        "àáâãäåæāăąçćĉċčďđèéêëēĕėęěĝğġģĥħìíîïĩīĭįıðĵķĸĺļľłñńņňŋòóôõöøōŏőœŕŗřśŝşšţťŧùúûüũūŭůűųÿŷŵýźżžþß"
        ;

const std::string HTTP_URI = std::string(
            "https?://"                                     // scheme
            "[" + DOMAIN_CHARS + "]+(?:\\.[" + DOMAIN_CHARS + "]+)*"  // host
            "(?::[0-9]+)?"                                  // port (optional)
            + URI_PATH_ABEMPTY +                            // path (optional)
            "(?:\\?" + URI_QUERY + ")?"                     // query (optional)
            "(?:#" + URI_FRAGMENT + ")?"                    // fragment (optional)
            );

const Regex LINKS_IN_MESSAGE_REGEX(
        "(^|[^a-zA-Z0-9])(" + HTTP_URI + ")"
        );

const Regex LINKS_IN_BRACKETS_REGEX(
        "\\((" + HTTP_URI + ")\\)"
        );

const Regex LINKS_BEFORE_PUNCTUATION_REGEX(
        "(" + HTTP_URI + ")([,;\\.:](?:$|[\\t\\n\\r ]))"
        );

const std::string KULLO_ADRESS_REGEX =
    "[a-zA-Z0-9]+"                   // username part
    "(?:"
        "[-\\._]"                    // separator: -._
        "[a-zA-Z0-9]+"               // username part
    ")*"
    "#"
    "(?:"
        "[a-zA-Z0-9]+(?:-[a-zA-Z0-9]+)*"
    "\\.)+"
    "[a-zA-Z][a-zA-Z0-9]*(?:-[a-zA-Z0-9]+)*"
;

const Regex KULLO_ADRESS_IN_MESSAGE_REGEX(
        "(^|[^-\\.#a-zA-Z0-9])(" + KULLO_ADRESS_REGEX + ")"
        );

const std::string KULLO_ADDRESS_SCHEME = "kulloInternal";

// Splits htmlIn in parts from <a ...> to </a> and non-link parts (rest).
// replaceCallback is called with all non-link parts as an argument.
// The result is the concatination of all link parts and replaceCallback of
// all non-link parts.
std::string replaceInNonLinkParts(
        const std::string &htmlIn,
        const std::function<std::string (const std::string)> &replaceCallback)
{
    std::string out;

    int htmlInPos = 0;
    bool foundLink;
    do {
        auto linkBegin = htmlIn.find("<a ", htmlInPos);

        if (linkBegin != std::string::npos)
        {
            const auto linkEnd = htmlIn.find("</a>", linkBegin)+4;
            const auto beforeLinkTextPart = std::string(htmlIn.begin()+htmlInPos, htmlIn.begin()+linkBegin);
            const auto linkTextPart = std::string(htmlIn.begin()+linkBegin, htmlIn.begin()+linkEnd);
            out += replaceCallback(beforeLinkTextPart);
            out += linkTextPart;
            htmlInPos = linkEnd;
            foundLink = true;
        }
        else
        {
            const auto rest = std::string(htmlIn.begin()+htmlInPos, htmlIn.end());
            out += replaceCallback(rest);
            foundLink = false;
        }

    } while (foundLink);

    return out;
}

}

std::string Strings::padLeft(const std::string &input, const size_t targetLength, const char paddingChar)
{
    if(targetLength <= input.size()) return input;

    std::string out = input;
    out.insert(out.begin(), targetLength-out.size(), paddingChar);
    return out;
}

std::string Strings::padRight(const std::string &input, const size_t targetLength, const char paddingChar)
{
    if(targetLength <= input.size()) return input;

    std::string out = input;
    out.insert(out.end(), targetLength-out.size(), paddingChar);
    return out;
}

std::string Strings::toLower(const std::string &s)
{
    std::string out;
    std::transform(s.cbegin(), s.cend(), std::back_inserter(out), ::tolower);
    return out;
}

std::string Strings::trim(const std::string &input)
{
    std::string s = input;

    // trim left
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    // trim right
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());

    return s;
}

std::string Strings::highlightWebLinks(const std::string &htmlIn)
{
    std::string out = htmlIn;

    out = replaceInNonLinkParts(out, [](const std::string &part) {
        return Regex::replace(part, LINKS_IN_BRACKETS_REGEX, "(<a href=\"$1\">$1</a>)");
    });

    out = replaceInNonLinkParts(out, [](const std::string &part) {
        return Regex::replace(part, LINKS_BEFORE_PUNCTUATION_REGEX, "<a href=\"$1\">$1</a>$2");
    });

    out = replaceInNonLinkParts(out, [](const std::string &part) {
        return Regex::replace(part, LINKS_IN_MESSAGE_REGEX, "$1<a href=\"$2\">$2</a>");
    });

    return out;
}

std::string Strings::highlightKulloAdresses(const std::string &htmlIn)
{
    std::string out = replaceInNonLinkParts(htmlIn, [](const std::string &part) {
        return Regex::replace(
                    part,
                    KULLO_ADRESS_IN_MESSAGE_REGEX,
                    "$1<a href=\"" + KULLO_ADDRESS_SCHEME + ":$2\">$2</a>");
    });
    return out;
}

std::string Strings::htmlEscape(const std::string &plaintext)
{
    std::string out;
    out.reserve(std::string::size_type(std::round(plaintext.size() * 1.10))); // prepare for 10 % overhead
    for(const char &c : plaintext)
    {
        switch(c)
        {
        case '&':  out.append("&amp;");  break;
        case '\"': out.append("&quot;"); break;
        case '<':  out.append("&lt;");   break;
        case '>':  out.append("&gt;");   break;
        default:   out.append(1, c);     break;
        }
    }
    return out;
}

std::string Strings::messageTextToHtml(const std::string &in, bool includeKulloAddresses)
{
    std::string out = htmlEscape(in);
    out = highlightWebLinks(out);
    if (includeKulloAddresses)
    {
        out = highlightKulloAdresses(out);
    }
    return out;
}

std::string Strings::formatReadable(std::int64_t value)
{
    auto isNegative = value < 0;
    std::string numberString = std::to_string(value);

    std::string out;

    std::vector<char> digits;
    if (isNegative)
    {
        digits = std::vector<char>(numberString.cbegin()+1, numberString.cend());
    }
    else
    {
        digits = std::vector<char>(numberString.cbegin(), numberString.cend());
    }

    // group digits with commas into blocks of 3
    while (digits.size() > 3)
    {
        out = "," + std::string(digits.end()-3, digits.end()) + out;
        digits.erase(digits.end()-3, digits.end());
    }

    auto digitsRemaining = std::string(digits.cbegin(), digits.cend());
    out = digitsRemaining + out;
    if (isNegative) out = "-" + out;

    return out;
}

}
}
