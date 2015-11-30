/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>
#include <vector>

namespace Kullo {
namespace Util {

std::vector<unsigned char> to_vector(const std::string &str = "");
std::string to_string(const std::vector<unsigned char> &vec);

}
}
