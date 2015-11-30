/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <sstream>
#include <string>

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Util {

class FormatString
{
public:
    static std::string padLeft(const std::string &input, const size_t targetLength, const char paddingChar = ' ');
    static std::string padRight(const std::string &input, const size_t targetLength, const char paddingChar = ' ');
    static std::string toLower(const std::string &s);
    static void trim(std::string &s);
    static void highlightLinks(std::string &target);
    static void escapeMessageText(std::string &target);
    static void messageTextToHtml(std::string &target);
    static std::string formatIntegerWithCommas(int value);
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
