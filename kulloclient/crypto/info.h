/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
