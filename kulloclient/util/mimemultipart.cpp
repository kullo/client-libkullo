/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/util/mimemultipart.h"

#include <cstdlib>

#include "kulloclient/util/assert.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/filesystem.h"

namespace Kullo {
namespace Util {

namespace {

// https://tools.ietf.org/html/rfc2046#section-5.1.1
std::string BCHARSNOSPACE =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "'()+_,-./:=?";
std::string BCHARS = BCHARSNOSPACE + " ";
std::string::size_type MIN_BOUNDARY_LENGTH = 1;
std::string::size_type MAX_BOUNDARY_LENGTH = 70;

std::string randomBoundary()
{
    std::string result(MAX_BOUNDARY_LENGTH, 'x');
    for (auto iter = result.begin(); iter != result.end(); ++iter)
    {
        // Yes, this is using rand instead of a cryptographically secure RNG.
        // Additionally, modular division doesn't lead to a perfectly uniform
        // distribution. However, this is good enough for our use case.
        *iter = BCHARSNOSPACE.at(std::rand() % BCHARSNOSPACE.size());
    }
    return result;
}

}

MimeMultipart::MimeMultipart()
    : boundary_(randomBoundary())
{}

MimeMultipart::MimeMultipart(const std::string &boundary)
    : boundary_(boundary)
{
    if ((boundary_.length() < MIN_BOUNDARY_LENGTH)
            || (boundary_.length() > MAX_BOUNDARY_LENGTH))
    {
        throw InvalidArgument(
                    std::string("Bad boundary length: ") +
                    std::to_string(boundary_.size()));
    }

    if (boundary_.back() == ' ')
    {
        throw InvalidArgument("Boundary must not end with a space");
    }
    for (auto bchar : boundary_)
    {
        if (BCHARS.find(bchar) == std::string::npos)
        {
            throw InvalidArgument(
                        std::string("Invalid char in boundary: ") +
                        std::to_string(bchar));
        }
    }
}

std::string MimeMultipart::boundary() const
{
    return boundary_;
}

std::string MimeMultipart::makePart(const std::string &name, const std::string &data)
{
    std::stringstream out;
    writePartHeader(out, name);
    out << data << "\r\n";
    return out.str();
}

std::string MimeMultipart::makePart(const std::string &name, std::uint32_t data)
{
    return makePart(name, std::to_string(data));
}

std::string MimeMultipart::makeFilePart(const std::string &name, const std::string &path)
{
    auto filename = Kullo::Util::Filesystem::filename(path);
    auto data = Kullo::Util::Filesystem::getContent(path);

    std::stringstream out;
    out << "--" << boundary_ << "\r\n"
        << "Content-Disposition: form-data; name=\"" << name << "\"; filename=\"" << filename << "\"\r\n"
        << "Content-Type: application/octet-stream\r\n"
        << "\r\n"
        << Kullo::Util::to_string(data) << "\r\n";
    return out.str();
}

void MimeMultipart::addPart(const std::string &name, const std::string &data)
{
    kulloAssert(!finalized_);
    out_ << makePart(name, data);
}

void MimeMultipart::addPart(const std::string &name, std::uint32_t data)
{
    kulloAssert(!finalized_);
    out_ << makePart(name, data);
}

void MimeMultipart::addPart(const std::string &name, const std::vector<unsigned char> &data)
{
    kulloAssert(!finalized_);
    writePartHeader(out_, name);
    out_.write(reinterpret_cast<const char *>(data.data()), data.size());
    out_ << "\r\n";
}

void MimeMultipart::addFilePart(const std::string &name, const std::string &path)
{
    kulloAssert(!finalized_);
    out_ << makeFilePart(name, path);
}

std::string MimeMultipart::toString()
{
    if (!finalized_)
    {
        out_ << "--" << boundary_ << "--\r\n";
        finalized_ = true;
    }

    return out_.str();
}

void MimeMultipart::writePartHeader(std::ostream &out, const std::string &name) const
{
    out << "--" << boundary_ << "\r\n"
        << "Content-Disposition: form-data; name=\"" << name << "\"\r\n"
        << "\r\n";
}

}
}
