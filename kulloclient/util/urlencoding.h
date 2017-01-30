/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>

namespace Kullo {
namespace Util {

class UrlEncoding
{
public:
    UrlEncoding() = delete;
    static std::string encode(const std::string &input);
};

}
}
