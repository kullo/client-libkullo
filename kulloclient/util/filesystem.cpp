/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/filesystem.h"

#include <exception>
#include <boost/filesystem.hpp>
#include <nowide/fstream.hpp>

#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Util {

std::string Filesystem::filename(const std::string &path)
{
    boost::filesystem::path p(path);
    return p.filename().string();
}

std::vector<unsigned char> Filesystem::getContent(const std::string &path)
{
    auto stream = makeIfstream(path);
    auto result = std::vector<unsigned char>(
                std::istreambuf_iterator<char>{*stream},
                std::istreambuf_iterator<char>{});
    if (stream->bad())
    {
        throw FilesystemError(std::string("Couldn't read content of ") + path);
    }
    return result;
}

void Filesystem::putContent(const std::string &path, const std::vector<unsigned char> &content)
{
    auto stream = makeOfstream(path);
    stream->write(
                reinterpret_cast<const char*>(content.data()),
                content.size());
    if (stream->fail())
    {
        throw FilesystemError(std::string("Failed while writing to ") + path);
    }
}

std::unique_ptr<std::istream> Filesystem::makeIfstream(const std::string &path)
{
    // we're using pointers here because we use a polymorphic return value
    auto stream = make_unique<nowide::ifstream>(path.c_str(), std::ios_base::binary);
    if (stream->fail())
    {
        throw FilesystemError(
                    std::string("Failed to open ") + path + " for reading.");
    }
    return std::move(stream);
}

std::unique_ptr<std::ostream> Filesystem::makeOfstream(const std::string &path)
{
    auto stream = make_unique<nowide::ofstream>(
                path.c_str(), std::ios_base::binary | std::ios_base::trunc);
    if (stream->fail())
    {
        throw FilesystemError(
                    std::string("Failed to open ") + path + " for writing.");
    }
    return std::move(stream);
}

}
}
