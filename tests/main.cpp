/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
