/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

namespace Kullo {
namespace Util {

class MimeMultipart
{
public:
    MimeMultipart(const std::string &boundary = "boundary_.oOo._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=");
    std::string boundary() const;

    std::string makePart(const std::string &name, const std::string &data);
    std::string makePart(const std::string &name, std::uint32_t data);
    std::string makeFilePart(const std::string &name, const std::string &path);
    void addPart(const std::string &name, const std::string &data);
    void addPart(const std::string &name, std::uint32_t data);
    void addPart(const std::string &name, const std::vector<unsigned char> &data);
    void addFilePart(const std::string &name, const std::string &path);
    std::string toString();

private:
    void writePartHeader(std::ostream &out, const std::string &name) const;

    const std::string boundary_;
    std::stringstream out_;
    bool finalized_ = false;
};

}
}
