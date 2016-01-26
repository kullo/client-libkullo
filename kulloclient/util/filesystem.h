/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <iosfwd>
#include <string>
#include <vector>
#include <memory>

namespace Kullo {
namespace Util {

class Filesystem
{
public:
    static std::string filename(const std::string &path);
    static std::vector<unsigned char> getContent(const std::string &path);
    static void putContent(const std::string &path, const std::vector<unsigned char> &content);
    static std::unique_ptr<std::istream> makeIfstream(const std::string &path);
    static std::unique_ptr<std::ostream> makeOfstream(const std::string &path);
};

}
}
