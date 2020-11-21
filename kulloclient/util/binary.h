/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <string>
#include <vector>

namespace Kullo {
namespace Util {

std::vector<unsigned char> to_vector(const std::string &str = "");
std::string to_string(const std::vector<unsigned char> &vec);

}
}
