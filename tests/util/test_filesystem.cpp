/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include <kulloclient/util/binary.h>
#include <kulloclient/util/filesystem.h>

#include "tests/kullotest.h"
#include "tests/testutil.h"

using namespace testing;
using namespace Kullo;

class Filesystem : public KulloTest
{
};

K_TEST_F(Filesystem, basename)
{
    std::string path;

#ifdef _WIN32 // is also defined on 64 bit...
    path = R"|(C:\Users\Public\Pictures\Creek.jpg)|";
    EXPECT_EQ("Creek.jpg", Util::Filesystem::filename(path));

    path = R"|(C:\Users\Public\Pictures\Space Dir\Creek.jpg)|";
    EXPECT_EQ("Creek.jpg", Util::Filesystem::filename(path));

    path = R"|(C:\Users\Public\noextension)|";
    EXPECT_EQ("noextension", Util::Filesystem::filename(path));
#else
    path = "/home/test/subdir/regular.file";
    EXPECT_EQ("regular.file", Util::Filesystem::filename(path));

    path = "/home/test/subdir/libkulloclient-test#kullo.net.db";
    EXPECT_EQ("libkulloclient-test#kullo.net.db", Util::Filesystem::filename(path));

    path = "/home/test/subdir/a file with spaces.txt";
    EXPECT_EQ("a file with spaces.txt", Util::Filesystem::filename(path));

    path = "/home/test/subdir/noextension";
    EXPECT_EQ("noextension", Util::Filesystem::filename(path));
#endif
}

K_TEST_F(Filesystem, folder)
{
    std::string path;

#ifdef _WIN32 // is also defined on 64 bit...
    path = R"|(C:\Users\Public\)|";
    EXPECT_EQ(".", Util::Filesystem::filename(path));

    path = R"|(C:\Users\Public\Sample Pictures\)|";
    EXPECT_EQ(".", Util::Filesystem::filename(path));
#endif

    path = "/home/test/folder";
    EXPECT_EQ("folder", Util::Filesystem::filename(path));

    path = "/home/test/folder/";
    EXPECT_EQ(".", Util::Filesystem::filename(path));

    path = "./relative/folder/";
    EXPECT_EQ(".", Util::Filesystem::filename(path));
}

K_TEST_F(Filesystem, readUtf8FilenameRelativeWithoutSpecialChar)
{
    {
        std::vector<unsigned char> data;
        auto path = TestUtil::assetPath() + "/filenames/test-a.txt";
        data = Util::Filesystem::getContent(path);
        EXPECT_THAT(data.size(), Gt(size_t{0}));
    }
}

K_TEST_F(Filesystem, readUtf8FilenameRelativeWithSpecialChar)
{
    {
        std::vector<unsigned char> data;
        auto path = TestUtil::assetPath() + "/filenames/test-'.txt";
        data = Util::Filesystem::getContent(path);
        EXPECT_THAT(data.size(), Gt(size_t{0}));
    }
    {
        std::vector<unsigned char> data;
        auto path = TestUtil::assetPath() + "/filenames/test-+.txt";
        data = Util::Filesystem::getContent(path);
        EXPECT_THAT(data.size(), Gt(size_t{0}));
    }
    {
        std::vector<unsigned char> data;
        auto path = TestUtil::assetPath() + "/filenames/test-&.txt";
        data = Util::Filesystem::getContent(path);
        EXPECT_THAT(data.size(), Gt(size_t{0}));
    }
}

K_TEST_F(Filesystem, readUtf8FilenameRelativeWithMultibyte)
{
    {
        std::vector<unsigned char> data;
        auto path = TestUtil::assetPath() + "/filenames/test-ä.txt";
        data = Util::Filesystem::getContent(path);
        EXPECT_THAT(data.size(), Gt(size_t{0}));
    }
    {
        std::vector<unsigned char> data;
        auto path = TestUtil::assetPath() + "/filenames/test-ß.txt";
        data = Util::Filesystem::getContent(path);
        EXPECT_THAT(data.size(), Gt(size_t{0}));
    }
}

K_TEST_F(Filesystem, readUtf8FilenameRelativeNonExistent)
{
    {
        auto path = TestUtil::assetPath() + "/filenames/test-ABC.txt";
        EXPECT_ANY_THROW(Util::Filesystem::getContent(path));
    }
    {
        auto path = TestUtil::assetPath() + "/filenames/test-ÄÖÜ.txt";
        EXPECT_ANY_THROW(Util::Filesystem::getContent(path));
    }
}

K_TEST_F(Filesystem, writeUtf8FilenameRelativeASCII)
{
    auto data = Util::to_vector("foo bar");

    {
        auto path = TestUtil::tempPath() + "/filesystem-123.txt";
        Util::Filesystem::putContent(path, data);
        EXPECT_THAT(Util::Filesystem::getContent(path), Eq(data));
    }
    {
        auto path = TestUtil::tempPath() + "/filesystem-abc.txt";
        Util::Filesystem::putContent(path, data);
        EXPECT_THAT(Util::Filesystem::getContent(path), Eq(data));
    }
}

K_TEST_F(Filesystem, writeUtf8FilenameRelativeMultibyte)
{
    auto data = Util::to_vector("foo bar");

    {
        auto path = TestUtil::tempPath() + "/filesystem-ä.txt";
        Util::Filesystem::putContent(path, data);
        EXPECT_THAT(Util::Filesystem::getContent(path), Eq(data));
    }
    {
        auto path = TestUtil::tempPath() + "/filesystem-ß.txt";
        Util::Filesystem::putContent(path, data);
        EXPECT_THAT(Util::Filesystem::getContent(path), Eq(data));
    }
}
