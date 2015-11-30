/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include <exception>

#include <kulloclient/crypto/symmetrickeygenerator.h>
#include <kulloclient/util/masterkey.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

class MasterKey : public KulloTest
{
};

K_TEST_F(MasterKey, validKeys)
{
    EXPECT_NO_THROW(Util::MasterKey(std::vector<std::string> {
                            "637694", "010702", "308148", "604439",
                            "273375", "064808", "558924", "550020",
                            "600759", "646869", "236885", "282145",
                            "137026", "568766", "438309", "322222"
                            }));
    EXPECT_NO_THROW(Util::MasterKey(std::vector<std::string> {
                            "444885", "329862", "495549", "397588",
                            "294777", "079459", "264960", "308700",
                            "464388", "155135", "362418", "125021",
                            "008839", "203554", "318667", "109363"
                            }));
    EXPECT_NO_THROW(Util::MasterKey(std::vector<std::string> {
                            "437368", "604843", "131268", "011841",
                            "227066", "125427", "119560", "083006",
                            "624619", "391615", "373043", "414466",
                            "132605", "641183", "242966", "377077"
                            }));
    EXPECT_NO_THROW(Util::MasterKey(std::vector<std::string> {
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455"
                            }));
    EXPECT_NO_THROW(Util::MasterKey(std::vector<std::string> {
                            "655357", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455"
                            }));
    EXPECT_NO_THROW(Util::MasterKey(std::vector<std::string> {
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000"
                            }));
}

K_TEST_F(MasterKey, wrongNumberOfBlocks)
{
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455"
                            }), std::invalid_argument);
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455", "123455"
                            }), std::invalid_argument);
}

K_TEST_F(MasterKey, wrongBlockLength)
{
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "1234557"
                            }), std::invalid_argument);
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "12345"
                            }), std::invalid_argument);
}

K_TEST_F(MasterKey, invalidChar)
{
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "w23455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455"
                            }), std::invalid_argument);
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "!23455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455"
                            }), std::invalid_argument);
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "ö23455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455",
                            "123455", "123455", "123455", "123455"
                            }), std::invalid_argument);
}

K_TEST_F(MasterKey, valueNotInRange)
{
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "999990", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000"
                            }), std::invalid_argument);
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "655360", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000"
                            }), std::invalid_argument);
}

K_TEST_F(MasterKey, luhnCheckSingleBlock)
{
    // wrong length
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock(""), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("0"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("00"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("0000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("00000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("0000000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("00000000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("000000000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("0000000000"), Eq(false));

    // wrong chars (end, middle, begin)
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("00000a"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("00000E"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("00000/"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("00000?"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("00000 "), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("00000\t"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("00000\n"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("000a00"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("000E00"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("000/00"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("000?00"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("000 00"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("000\t00"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("000\n00"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("a00"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("E00000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("/00000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("?00000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock(" 00000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("\t00000"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("\n00000"), Eq(false));

    // correct chars
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("000000"), Eq(true));

    // Correct block modified at the end
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170530"), Eq(true));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170531"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170532"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170533"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170534"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170535"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170536"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170537"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170538"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170539"), Eq(false));

    // Correct block modified at the beginning
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("170530"), Eq(true));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("070530"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("270530"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("370530"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("470530"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("570530"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("670530"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("770530"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("870530"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("970530"), Eq(false));

    // Range (16 bit ~ [0, 65535])
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("655357"), Eq(true));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("655365"), Eq(false));
    EXPECT_THAT(Util::MasterKey::isPlausibleSingleBlock("999995"), Eq(false));
}

K_TEST_F(MasterKey, luhnCheckInvalid)
{
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "437367", "604843", "131268", "011841",
                            "227066", "125427", "119560", "083006",
                            "624619", "391615", "373043", "414466",
                            "132605", "641183", "242966", "377077"
                            }), std::invalid_argument);
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "437369", "604843", "131268", "011841",
                            "227066", "125427", "119560", "083006",
                            "624619", "391615", "373043", "414466",
                            "132605", "641183", "242966", "377077"
                            }), std::invalid_argument);
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "000001", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000"
                            }), std::invalid_argument);
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "000000", "000000", "000000", "000000",
                            "000000", "000001", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000"
                            }), std::invalid_argument);
    EXPECT_THROW(Util::MasterKey(std::vector<std::string> {
                            "000000", "000000", "000000", "000000",
                            "000000", "000001", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000001"
                            }), std::invalid_argument);
}

K_TEST_F(MasterKey, version)
{
    Util::MasterKey keyVersion1(std::string(
                                  "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                                  "Version: 1 (256 bit)\n"
                                  "\n"
                                  "009167 186544 520379 167502\n"
                                  "092114 170530 357103 015669\n"
                                  "422915 084699 505677 309062\n"
                                  "407924 293407 624445 483701\n"
                                  "-----END KULLO PRIVATE MASTER KEY-----\n"));
    EXPECT_EQ(std::uint32_t{1}, keyVersion1.version());

    Util::MasterKey keyVersion7(std::string(
                                  "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                                  "Version: 7 (256 bit)\n"
                                  "\n"
                                  "009167 186544 520379 167502\n"
                                  "092114 170530 357103 015669\n"
                                  "422915 084699 505677 309062\n"
                                  "407924 293407 624445 483701\n"
                                  "-----END KULLO PRIVATE MASTER KEY-----\n"));
    EXPECT_EQ(std::uint32_t{7}, keyVersion7.version());

    EXPECT_EQ(std::uint32_t{1}, Util::MasterKey(std::vector<std::string> {
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000"
                            }).version());
    EXPECT_EQ(std::uint32_t{1}, Util::MasterKey(std::vector<std::string> {
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000"
                            }, 1).version());
    EXPECT_EQ(std::uint32_t{9}, Util::MasterKey(std::vector<std::string> {
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000"
                            }, 9).version());
    EXPECT_NE(std::uint32_t{9}, Util::MasterKey(std::vector<std::string> {
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000",
                            "000000", "000000", "000000", "000000"
                            }).version());
}

K_TEST_F(MasterKey, extractDataBlocks)
{
    Util::MasterKey keyTextVersion1(std::string(
                "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                "Version: 1 (256 bit)\n"
                "\n"
                "009167 186544 520379 167502\n"
                "092114 170530 357103 015669\n"
                "422915 084699 505677 309062\n"
                "407924 293407 624445 483701\n"
                "-----END KULLO PRIVATE MASTER KEY-----\n"
                ));
    std::vector<std::string> blocks {
               "009167", "186544", "520379", "167502",
               "092114", "170530", "357103", "015669",
               "422915", "084699", "505677", "309062",
               "407924", "293407", "624445", "483701"
               };
    EXPECT_EQ(blocks, keyTextVersion1.dataBlocks());
}

K_TEST_F(MasterKey, extractDataBlocksIncompleteKey)
{
    // full key
    EXPECT_NO_THROW(Util::MasterKey(std::string(
            "----BEGIN KULLO PRIVATE MASTER KEY-----\n"
            "Version: 1 (256 bit)\n"
            "\n"
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            "-----END KULLO PRIVATE MASTER KEY-----\n"
            )));
    // begin tag missing
    EXPECT_NO_THROW(Util::MasterKey(std::string(
            "Version: 1 (256 bit)\n"
            "\n"
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            "-----END KULLO PRIVATE MASTER KEY-----\n"
            )));
    // begin tag broken
    EXPECT_NO_THROW(Util::MasterKey(std::string(
            "-BEGIN KULLO PRIVATE MASTER KEY-\n"
            "Version: 1 (256 bit)\n"
            "\n"
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            "-----END KULLO PRIVATE MASTER KEY-----\n"
            )));
    EXPECT_NO_THROW(Util::MasterKey(std::string(
            "BEGIN KULLO PRIVATE MASTER KEY\n"
            "Version: 1 (256 bit)\n"
            "\n"
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            "-----END KULLO PRIVATE MASTER KEY-----\n"
            )));
    // begin tag broken too much
    EXPECT_THROW(Util::MasterKey(std::string(
            "GIN KULLO PRIVATE MASTER KEY\n"
            "Version: 1 (256 bit)\n"
            "\n"
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            "-----END KULLO PRIVATE MASTER KEY-----\n"
            )), std::invalid_argument);
    // end tag broken too much ("MASTER KEY" -> "MASTERKEY")
    EXPECT_THROW(Util::MasterKey(std::string(
            "----BEGIN KULLO PRIVATE MASTER KEY-----\n"
            "Version: 1 (256 bit)\n"
            "\n"
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            "-----END KULLO PRIVATE MASTERKEY-----\n"
            )), std::invalid_argument);
    // end tag missing
    EXPECT_NO_THROW(Util::MasterKey(std::string(
            "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
            "Version: 1 (256 bit)\n"
            "\n"
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            )));
    // end tag broken
    EXPECT_NO_THROW(Util::MasterKey(std::string(
            "----BEGIN KULLO PRIVATE MASTER KEY-----\n"
            "Version: 1 (256 bit)\n"
            "\n"
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            "-----END KULLO PRIVATE MASTER KEY--\n"
            )));
    EXPECT_NO_THROW(Util::MasterKey(std::string(
            "----BEGIN KULLO PRIVATE MASTER KEY-----\n"
            "Version: 1 (256 bit)\n"
            "\n"
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            "-----END KULLO PRIVATE MASTER KEY\n"
            )));
    // begin and end tag missing
    EXPECT_NO_THROW(Util::MasterKey(std::string(
            "Version: 1 (256 bit)\n"
            "\n"
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            )));
    // data blocks only
    EXPECT_NO_THROW(Util::MasterKey(std::string(
            "009167 186544 520379 167502\n"
            "092114 170530 357103 015669\n"
            "422915 084699 505677 309062\n"
            "407924 293407 624445 483701\n"
            )));
    Util::MasterKey keyTextVersion1(std::string(
                "009167 186544 520379 167502\n"
                "092114 170530 357103 015669\n"
                "422915 084699 505677 309062\n"
                "407924 293407 624445 483701"
                ));
    std::vector<std::string> blocks {
               "009167", "186544", "520379", "167502",
               "092114", "170530", "357103", "015669",
               "422915", "084699", "505677", "309062",
               "407924", "293407", "624445", "483701"
               };
    EXPECT_EQ(blocks, keyTextVersion1.dataBlocks());
}

K_TEST_F(MasterKey, invalidHeaders)
{
    // no colon after "Version"
    EXPECT_THROW(Util::MasterKey("-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                                 "Version 1 (256 bit)\n"
                                 "\n"
                                 "009167 186544 520379 167502\n"
                                 "092114 170530 357103 015669\n"
                                 "422915 084699 505677 309062\n"
                                 "407924 293407 624445 483701\n"
                                 "-----END KULLO PRIVATE MASTER KEY-----\n"
                                 ), std::invalid_argument);

    // space between "Version" and colon
    EXPECT_THROW(Util::MasterKey("-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                                 "Version : 1 (256 bit)\n"
                                 "\n"
                                 "009167 186544 520379 167502\n"
                                 "092114 170530 357103 015669\n"
                                 "422915 084699 505677 309062\n"
                                 "407924 293407 624445 483701\n"
                                 "-----END KULLO PRIVATE MASTER KEY-----\n"
                                 ), std::invalid_argument);

    // Unknown keyword "Date"
    EXPECT_THROW(Util::MasterKey("-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                                 "Date: 2014-09-22\n"
                                 "\n"
                                 "009167 186544 520379 167502\n"
                                 "092114 170530 357103 015669\n"
                                 "422915 084699 505677 309062\n"
                                 "407924 293407 624445 483701\n"
                                 "-----END KULLO PRIVATE MASTER KEY-----\n"
                                 ), std::invalid_argument);
}

K_TEST_F(MasterKey, toPem)
{
    Util::MasterKey key(std::vector<std::string> {
                       "009167", "186544", "520379", "167502",
                       "092114", "170530", "357103", "015669",
                       "422915", "084699", "505677", "309062",
                       "407924", "293407", "624445", "483701"
                       });
    std::string keyAsString(
                "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                "Version: 1 (256 bit)\n"
                "\n"
                "009167 186544 520379 167502\n"
                "092114 170530 357103 015669\n"
                "422915 084699 505677 309062\n"
                "407924 293407 624445 483701\n"
                "-----END KULLO PRIVATE MASTER KEY-----\n"
                );
    EXPECT_EQ(key.toPem(), keyAsString);
}

K_TEST_F(MasterKey, keyEqualsItself)
{
    Util::MasterKey newKey = Util::MasterKey::makeRandom();
    EXPECT_EQ(newKey, newKey);
}

K_TEST_F(MasterKey, unequal)
{
    Util::MasterKey keyVersion1(std::string(
                                  "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                                  "Version: 1 (256 bit)\n"
                                  "\n"
                                  "009167 186544 520379 167502\n"
                                  "092114 170530 357103 015669\n"
                                  "422915 084699 505677 309062\n"
                                  "407924 293407 624445 483701\n"
                                  "-----END KULLO PRIVATE MASTER KEY-----\n"));
    Util::MasterKey keyVersion2(std::string(
                                  "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                                  "Version: 2 (256 bit)\n"
                                  "\n"
                                  "009167 186544 520379 167502\n"
                                  "092114 170530 357103 015669\n"
                                  "422915 084699 505677 309062\n"
                                  "407924 293407 624445 483701\n"
                                  "-----END KULLO PRIVATE MASTER KEY-----\n"));
    EXPECT_NE(keyVersion1, keyVersion2); // different version

    Util::MasterKey keyVersion3(std::string(
                                  "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                                  "Version: 1 (256 bit)\n"
                                  "\n"
                                  "407924 293407 624445 483701\n"
                                  "009167 186544 520379 167502\n"
                                  "092114 170530 357103 015669\n"
                                  "422915 084699 505677 309062\n"
                                  "-----END KULLO PRIVATE MASTER KEY-----\n"));
    EXPECT_NE(keyVersion1, keyVersion3); // different key
}

K_TEST_F(MasterKey, makeRandomHasCorrectSize)
{
    Util::MasterKey newKey = Util::MasterKey::makeRandom();
    EXPECT_THAT(newKey.toVector().size(), Eq(Crypto::SymmetricKeyGenerator::MASTER_KEY_BITS / 8));
}

K_TEST_F(MasterKey, makeRandomHasCorrectVersion)
{
    Util::MasterKey newKey = Util::MasterKey::makeRandom();
    EXPECT_THAT(newKey.version(), Eq(std::uint32_t{1}));
}

K_TEST_F(MasterKey, makeRandomGeneratesDifferentKeys)
{
    Util::MasterKey newKey1 = Util::MasterKey::makeRandom();
    Util::MasterKey newKey2 = Util::MasterKey::makeRandom();
    Util::MasterKey newKey3 = Util::MasterKey::makeRandom();
    Util::MasterKey newKey4 = Util::MasterKey::makeRandom();
    Util::MasterKey newKey5 = Util::MasterKey::makeRandom();
    EXPECT_NE(newKey1, newKey2);
    EXPECT_NE(newKey1, newKey2);
    EXPECT_NE(newKey1, newKey3);
    EXPECT_NE(newKey1, newKey4);
    EXPECT_NE(newKey1, newKey5);
    EXPECT_NE(newKey2, newKey3);
    EXPECT_NE(newKey2, newKey4);
    EXPECT_NE(newKey2, newKey5);
    EXPECT_NE(newKey3, newKey4);
    EXPECT_NE(newKey3, newKey4);
    EXPECT_NE(newKey4, newKey5);
}


// Add compression test as soon as boost is available
