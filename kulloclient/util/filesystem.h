/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>
#include <vector>

namespace Kullo {
namespace Util {

class Filesystem
{
public:
    static std::string filename(const std::string &path);
    static std::vector<unsigned char> getContent(const std::string &path);
    static void putContent(const std::string &path, const std::vector<unsigned char> &content);
};

}
}
