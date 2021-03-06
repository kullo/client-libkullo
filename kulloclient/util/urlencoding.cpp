/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/util/urlencoding.h"

#include <cctype>
#include <iomanip>
#include <ios>
#include <sstream>

namespace Kullo {
namespace Util {

std::string UrlEncoding::encode(const std::string &input)
{
    std::stringstream result;
    // use hex encoding and fill with zeroes when streaming integers
    result << std::hex << std::setfill('0');

    for (const auto chr : input)
    {
        if (std::isalnum(chr) || chr == '-' || chr == '.'
                || chr == '_' || chr == '~')
        {
            result << chr;
        }
        else
        {
            result << '%'
                   // print char as uppercase hex integer (two chars wide)
                   << std::uppercase << std::setw(2)
                   << static_cast<int>(chr)
                   << std::nouppercase;
        }
    }

    return result.str();
}

}
}
