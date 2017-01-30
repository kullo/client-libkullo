/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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

K_TEST_F(Filesystem, filesize)
{
    const auto inFilepaths = std::vector<std::string>{
        TestUtil::assetPath() + "/filenames/test-a.txt",
        TestUtil::assetPath() + "/filenames/test-ä.txt",
        TestUtil::assetPath() + "/filenames/test-ß.txt",
        TestUtil::assetPath() + "/filenames/test-'.txt",
        TestUtil::assetPath() + "/filenames/test-&.txt",
        TestUtil::assetPath() + "/filenames/test-+.txt",
    };

    for (const auto &inFilepath : inFilepaths)
    {
        auto size = Util::Filesystem::fileSize(inFilepath);
        EXPECT_THAT(size, Gt(0u));
    }
}

K_TEST_F(Filesystem, exists)
{
    const auto inFilepaths = std::vector<std::string>{
        TestUtil::assetPath() + "/filenames/test-a.txt",
        TestUtil::assetPath() + "/filenames/test-ä.txt",
        TestUtil::assetPath() + "/filenames/test-ß.txt",
        TestUtil::assetPath() + "/filenames/test-'.txt",
        TestUtil::assetPath() + "/filenames/test-&.txt",
        TestUtil::assetPath() + "/filenames/test-+.txt",
    };

    for (const auto &inFilepath : inFilepaths)
    {
        EXPECT_THAT(Util::Filesystem::exists(inFilepath), Eq(true));
    }
}

K_TEST_F(Filesystem, isRegularFile)
{
    const auto inFilepaths = std::vector<std::string>{
        TestUtil::assetPath() + "/filenames/test-a.txt",
        TestUtil::assetPath() + "/filenames/test-ä.txt",
        TestUtil::assetPath() + "/filenames/test-ß.txt",
        TestUtil::assetPath() + "/filenames/test-'.txt",
        TestUtil::assetPath() + "/filenames/test-&.txt",
        TestUtil::assetPath() + "/filenames/test-+.txt",
    };

    for (const auto &inFilepath : inFilepaths)
    {
        EXPECT_THAT(Util::Filesystem::isRegularFile(inFilepath), Eq(true));
    }
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

K_TEST_F(Filesystem, writeReadLatin1Filenames)
{
    // lower half of latin 1 table minus NBSP and SHY
    const auto latin1_chars = std::vector<std::string> {
            "_", "¡", "¢", "£", "¤", "¥", "¦", "§", "¨", "©", "ª", "«", "¬", "®", "_", "¯",
            "°", "±", "²", "³", "´", "µ", "¶", "·", "¸", "¹", "º", "»", "¼", "½", "¾", "¿",
            "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "Ç", "È", "É", "Ê", "Ë", "Ì", "Í", "Î", "Ï",
            "Ð", "Ñ", "Ò", "Ó", "Ô", "Õ", "Ö", "×", "Ø", "Ù", "Ú", "Û", "Ü", "Ý", "Þ", "ß",
            "à", "á", "â", "ã", "ä", "å", "æ", "ç", "è", "é", "ê", "ë", "ì", "í", "î", "ï",
            "ð", "ñ", "ò", "ó", "ô", "õ", "ö", "÷", "ø", "ù", "ú", "û", "ü", "ý", "þ", "ÿ"
            };

    const auto data = Util::to_vector("foo2");

    for (const auto &c : latin1_chars)
    {
        auto path = TestUtil::tempPath() + "/filesystem-" + c + ".txt";
        Util::Filesystem::putContent(path, data);
        EXPECT_THAT(Util::Filesystem::getContent(path), Eq(data));
    }
}
