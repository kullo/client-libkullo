/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/protocol/httpstructs.h"

#include "kulloclient/util/base64.h"

namespace Kullo {
namespace Protocol {

bool IdLastModified::operator==(const IdLastModified &rhs) const
{
    return id == rhs.id && lastModified == rhs.lastModified;
}

bool IdLastModified::operator<(const IdLastModified &rhs) const
{
    return this->lastModified < rhs.lastModified;
}

std::string Message::toString()
{
    auto dateReceivedStr = dateReceived ? dateReceived->toString() : "";

    std::string ret;
    ret += "id:\t" + std::to_string(id) + "\n";
    ret += "lastModified:\t" + std::to_string(lastModified) + "\n";
    ret += "deleted:\t" + std::to_string(deleted) + "\n";
    ret += "received:\t" + dateReceivedStr + "\n";
    ret += "meta:\t" + Util::Base64::encode(meta) + "\n";
    ret += "keySafe:\t" + Util::Base64::encode(keySafe) + "\n";
    ret += "content:\t" + Util::Base64::encode(content) + "\n";
    ret += "hasAttachments:\t" + std::to_string(hasAttachments) + "\n";
    return ret;
}

bool Message::operator==(const Message &rhs) const
{
    return
            IdLastModified::operator==(rhs) &&
            deleted == rhs.deleted &&
            dateReceived == rhs.dateReceived &&
            meta == rhs.meta &&
            keySafe == rhs.keySafe &&
            content == rhs.content &&
            hasAttachments == rhs.hasAttachments;
}

}
}
