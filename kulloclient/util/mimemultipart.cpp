/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/mimemultipart.h"

#include "kulloclient/util/assert.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/filesystem.h"

namespace Kullo {
namespace Util {

MimeMultipart::MimeMultipart(const std::string &boundary)
    : boundary_(boundary)
{
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
