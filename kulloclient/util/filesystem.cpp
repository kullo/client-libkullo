/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/util/filesystem.h"

#include <exception>
#include <boost/filesystem.hpp>
#include <nowide/fstream.hpp>

#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Util {

#ifdef _WIN32 // is also defined on 64 bit...
    #include <nowide/convert.hpp>
    #define K_WIDEN_IF_NEEDED(path) nowide::widen(path)
#else
    #define K_WIDEN_IF_NEEDED(path) path
#endif

std::string Filesystem::filename(const std::string &path)
{
    boost::filesystem::path p(path);
    return p.filename().string();
}

bool Filesystem::exists(const std::string &path)
{
    return boost::filesystem::exists(K_WIDEN_IF_NEEDED(path));
}

bool Filesystem::isRegularFile(const std::string &path)
{
    return boost::filesystem::is_regular_file(K_WIDEN_IF_NEEDED(path));
}

size_t Filesystem::fileSize(const std::string &path)
{
    try
    {
        return boost::filesystem::file_size(K_WIDEN_IF_NEEDED(path));
    }
    catch (boost::filesystem::filesystem_error &)
    {
        std::throw_with_nested(FilesystemError("Couldn't get size of " + path));
    }
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
