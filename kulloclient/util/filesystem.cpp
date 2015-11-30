/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/filesystem.h"

#include <exception>
#include <boost/filesystem.hpp>
#include <nowide/fstream.hpp>

namespace Kullo {
namespace Util {

std::string Filesystem::filename(const std::string &path)
{
    boost::filesystem::path p(path);
    return p.filename().string();
}

std::vector<unsigned char> Filesystem::getContent(const std::string &path)
{
    nowide::ifstream file;
    file.exceptions(~std::ios_base::goodbit);
    file.open(path.c_str(), std::ios_base::binary | std::ios_base::in);
    auto out = std::vector<unsigned char>(std::istreambuf_iterator<char>{file},
                                          std::istreambuf_iterator<char>{});

    return out;
}

void Filesystem::putContent(const std::string &path, const std::vector<unsigned char> &content)
{
    nowide::ofstream file;
    file.exceptions(~std::ios_base::goodbit);
    file.open(path.c_str(),
              std::ios_base::binary |
              std::ios_base::out |
              std::ios_base::trunc);
    file.write(reinterpret_cast<const char*>(content.data()), content.size());
}

}
}
