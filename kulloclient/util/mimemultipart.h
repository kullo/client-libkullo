/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
    MimeMultipart();
    MimeMultipart(const std::string &boundary);
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
