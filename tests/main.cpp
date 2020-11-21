/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <iostream>

#include "tests/commonmain.h"
#include "tests/testutil.h"

std::atomic<int> TestUtil::testCounter_(0);
std::string TestUtil::tempPath_;
std::string TestUtil::assetPath_;

int main(int argc, char** argv)
{
    return commonMain("libkullo", NeedAssets::Yes, argc, argv);
}
