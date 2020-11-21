/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "tests/kullotest.h"
#include "tests/testutil.h"

#include <sstream>
#include <kulloclient/util/exceptions.h>
#include <kulloclient/util/mimemultipart.h>

using namespace testing;

class Multipart : public KulloTest
{
public:
    const std::string TEST_FILES_DIR = TestUtil::assetPath() + "/multipart";
    const std::string BOUNDARY =
            "boundary_.aBa._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=";
};

TEST_F(Multipart, validBoundaries)
{
    Kullo::Util::MimeMultipart mp1("x");
    EXPECT_THAT(mp1.boundary(), Eq("x"));

    Kullo::Util::MimeMultipart mp2("3456789");
    EXPECT_THAT(mp2.boundary(), Eq("3456789"));

    Kullo::Util::MimeMultipart mp3(BOUNDARY);
    EXPECT_THAT(mp3.boundary(), Eq(BOUNDARY));
}

TEST_F(Multipart, invalidBoundaries)
{
    EXPECT_THROW(Kullo::Util::MimeMultipart(""),
                 Kullo::Util::InvalidArgument);

    EXPECT_THROW(Kullo::Util::MimeMultipart(" "),
                 Kullo::Util::InvalidArgument);

    EXPECT_THROW(Kullo::Util::MimeMultipart("x "),
                 Kullo::Util::InvalidArgument);
}

TEST_F(Multipart, randomBoundary)
{
    auto boundary1 = Kullo::Util::MimeMultipart().boundary();
    auto boundary2 = Kullo::Util::MimeMultipart().boundary();

    EXPECT_THAT(boundary1, Not(Eq(boundary2)));

    // check validity of generated boundaries
    EXPECT_NO_THROW(Kullo::Util::MimeMultipart(boundary1));
    EXPECT_NO_THROW(Kullo::Util::MimeMultipart(boundary2));
}

TEST_F(Multipart, makeEmptyPart)
{
    const std::string name = "foo";
    Kullo::Util::MimeMultipart uut;
    auto part = uut.makePart(name, "");

    std::ostringstream expected;
    expected << "--" << uut.boundary() << "\r\n"
             << "Content-Disposition: form-data; name=\"" << name << "\"\r\n"
             << "\r\n"
             << "\r\n";
    EXPECT_THAT(part, Eq(expected.str()));
}

TEST_F(Multipart, addEmptyPart)
{
    const std::string name = "foo";
    Kullo::Util::MimeMultipart uut;
    uut.addPart(name, std::vector<unsigned char>());

    std::ostringstream expected;
    expected << "--" << uut.boundary() << "\r\n"
             << "Content-Disposition: form-data; name=\"" << name << "\"\r\n"
             << "\r\n"
             << "\r\n"
             << "--" << uut.boundary() << "--\r\n";
    EXPECT_THAT(uut.toString(), Eq(expected.str()));
}

TEST_F(Multipart, makeStringPart)
{
    const auto EXP_PROD =
            "--boundary_.aBa._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=\r\n"
            "Content-Disposition: form-data; name=\"prod\"\r\n"
            "\r\n"
            "test\r\n";
    const auto EXP_VERSION =
            "--boundary_.aBa._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=\r\n"
            "Content-Disposition: form-data; name=\"ver\"\r\n"
            "\r\n"
            "0.0.1\r\n";
    const auto EXP_COMMENTS =
            "--boundary_.aBa._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=\r\n"
            "Content-Disposition: form-data; name=\"comments\"\r\n"
            "\r\n"
            "Das Programm\n"
            "ist leider\n"
            "kaputt gegangen :(\r\n";
    const auto EXP_CTIME =
            "--boundary_.aBa._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=\r\n"
            "Content-Disposition: form-data; name=\"ctime\"\r\n"
            "\r\n"
            "123\r\n";

    Kullo::Util::MimeMultipart mp(BOUNDARY);
    EXPECT_THAT(mp.makePart("prod", "test"), Eq(EXP_PROD));
    EXPECT_THAT(mp.makePart("ver", "0.0.1"), Eq(EXP_VERSION));
    EXPECT_THAT(mp.makePart("comments", "Das Programm\n"
                                            "ist leider\n"
                                            "kaputt gegangen :("),
                Eq(EXP_COMMENTS));
    EXPECT_THAT(mp.makePart("ctime", 123), Eq(EXP_CTIME));
}

TEST_F(Multipart, makeFilePart)
{
    const auto EXP_FILE_SIMPLE =
            "--boundary_.aBa._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=\r\n"
            "Content-Disposition: form-data; name=\"file_simple\"; filename=\"file_simple.txt\"\r\n"
            "Content-Type: application/octet-stream\r\n"
            "\r\n"
            "123.45\n"
            "\r\n";
    const auto EXP_FILE_UTF8 =
            "--boundary_.aBa._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=\r\n"
            "Content-Disposition: form-data; name=\"file_utf8\"; filename=\"file_utf8.txt\"\r\n"
            "Content-Type: application/octet-stream\r\n"
            "\r\n"
            "öäüßß\n"
            "\r\n";
    const auto EXP_FILE_EMPTY =
            "--boundary_.aBa._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=\r\n"
            "Content-Disposition: form-data; name=\"file_empty\"; filename=\"file_empty.txt\"\r\n"
            "Content-Type: application/octet-stream\r\n"
            "\r\n"
            "\r\n";
    const auto EXP_FILE_SPACE =
            "--boundary_.aBa._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=\r\n"
            "Content-Disposition: form-data; name=\"file_space\"; filename=\"file space.txt\"\r\n"
            "Content-Type: application/octet-stream\r\n"
            "\r\n"
            "\r\n";
    const auto EXP_FILE_SPECIAL =
            "--boundary_.aBa._MTA1NTM3NjEwMg==Mjc5OTIzODY0MTI0MTc2NjY=\r\n"
            "Content-Disposition: form-data; name=\"file_special\"; filename=\"file_special_öß.txt\"\r\n"
            "Content-Type: application/octet-stream\r\n"
            "\r\n"
            "ok\n"
            "\r\n";

    Kullo::Util::MimeMultipart mp(BOUNDARY);
    EXPECT_THAT(mp.makeFilePart("file_simple", TEST_FILES_DIR + "/file_simple.txt"),
                Eq(EXP_FILE_SIMPLE));
    EXPECT_THAT(mp.makeFilePart("file_utf8", TEST_FILES_DIR + "/file_utf8.txt"),
                Eq(EXP_FILE_UTF8));
    EXPECT_THAT(mp.makeFilePart("file_empty", TEST_FILES_DIR + "/file_empty.txt"),
                Eq(EXP_FILE_EMPTY));
    EXPECT_THAT(mp.makeFilePart("file_space", TEST_FILES_DIR + "/file space.txt"),
                Eq(EXP_FILE_SPACE));
    EXPECT_THAT(mp.makeFilePart("file_special", TEST_FILES_DIR + "/file_special_öß.txt"),
                Eq(EXP_FILE_SPECIAL));
}

TEST_F(Multipart, combo)
{
    const auto EX_RESULT =
            "--boundary_.oOo._MTQwNzg0MjY1Mw==NjgxNDk0MDI1OTkxOTE5NjE0\r\n"
            "Content-Disposition: form-data; name=\"prod\"\r\n\r\ntest\r\n"
            "--boundary_.oOo._MTQwNzg0MjY1Mw==NjgxNDk0MDI1OTkxOTE5NjE0\r\n"
            "Content-Disposition: form-data; name=\"ver\"\r\n\r\n0.0.1\r\n"
            "--boundary_.oOo._MTQwNzg0MjY1Mw==NjgxNDk0MDI1OTkxOTE5NjE0\r\n"
            "Content-Disposition: form-data; name=\"guid\"\r\n\r\n12\r\n"
            "--boundary_.oOo._MTQwNzg0MjY1Mw==NjgxNDk0MDI1OTkxOTE5NjE0\r\n"
            "Content-Disposition: form-data; name=\"ptime\"\r\n\r\n4500\r\n"
            "--boundary_.oOo._MTQwNzg0MjY1Mw==NjgxNDk0MDI1OTkxOTE5NjE0\r\n"
            "Content-Disposition: form-data; name=\"ctime\"\r\n\r\n285389\r\n"
            "--boundary_.oOo._MTQwNzg0MjY1Mw==NjgxNDk0MDI1OTkxOTE5NjE0\r\n"
            "Content-Disposition: form-data; name=\"email\"\r\n\r\n"
            "test@example.com\r\n"
            "--boundary_.oOo._MTQwNzg0MjY1Mw==NjgxNDk0MDI1OTkxOTE5NjE0\r\n"
            "Content-Disposition: form-data; name=\"comments\"\r\n\r\n"
            "Da ist was\nabgest\xc3\xbcrzt :/\r\n"
            "--boundary_.oOo._MTQwNzg0MjY1Mw==NjgxNDk0MDI1OTkxOTE5NjE0\r\n"
            "Content-Disposition: form-data; name=\"upload_file_minidump\"; "
            "filename=\"file_simple.txt\"\r\n"
            "Content-Type: application/octet-stream\r\n\r\n123.45\n\r\n"
            "--boundary_.oOo._MTQwNzg0MjY1Mw==NjgxNDk0MDI1OTkxOTE5NjE0--\r\n";

    Kullo::Util::MimeMultipart mp("boundary_.oOo._MTQwNzg0MjY1Mw==NjgxNDk0MDI1OTkxOTE5NjE0");
    mp.addPart("prod", "test");
    mp.addPart("ver", "0.0.1");
    mp.addPart("guid", "12");
    mp.addPart("ptime", 4500);
    mp.addPart("ctime", 285389);
    mp.addPart("email", "test@example.com");
    mp.addPart("comments", "Da ist was\n"
                               "abgestürzt :/");
    mp.addFilePart("upload_file_minidump", TEST_FILES_DIR + "/file_simple.txt");

    EXPECT_THAT(mp.toString(), Eq(EX_RESULT));

    // make sure toString() returns the same data multiple times
    EXPECT_THAT(mp.toString(), Eq(EX_RESULT));
}
