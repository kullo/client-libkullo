/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <map>
#include <string>

namespace Kullo {
namespace Crypto {

class Info
{
public:
    static std::map<std::string, std::string> getImplementationInfos();
};

}
}
