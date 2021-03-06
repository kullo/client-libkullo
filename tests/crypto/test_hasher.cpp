/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/crypto/hasher.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/hex.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

class HasherTest : public KulloTest
{
protected:
    // using test vectors from http://csrc.nist.gov/groups/STM/cavp/index.html#03
    // SHA-256 lengths: 0, 8, 16, 512, 10808 bit
    // SHA-512 lengths: 0, 8, 16, 1024, 10528 bit
    const std::vector<std::string> SHA_256_TEST_VECTORS = {
        "",
        "d3",
        "11af",
        "5a86b737eaea8ee976a0a24da63e7ed7eefad18a101c1211e2b3650c5187c2a8a65054720"
        "8251f6d4237e661c7bf4c77f335390394c37fa1a9f9be836ac28509",
        "9020918aad4ebe24bfbad9f9109325d09ef520bd79ba08986d949fade1592cb5ff9dc2061"
        "586c4063bdca9e53760fd8c9d5fa8d03b8673ecb3f8c82e6a9eb9f0a1be45cae2d0d6069e"
        "8d0d541448c2bf748147e045b8ed52047fca660ed3b917c0aca140dcd3fb0c2ef48eae70f"
        "47d536c84845560f77fb2a6502cbc94a03112a28f61ceca383b00353ab35c130b362fcb90"
        "e89854eb30f4e295769ac6ac2bc98f8e0ade76a69ecaf98605c4c536f33bd9ccfa0fe93d0"
        "800007331676aa0ae24d1d126d7a6c62d53c3010b4f4e1dbe5fe0614223e6950fbe4814e4"
        "8a4923c30baf813c212340ef81dad24d6575679e832677483c159a4e1702a0176d2bde716"
        "670c6d524b5bbed3d8823536f03bd9c8ff43495c33cf5ccf1753e5277d878c01d5dc77847"
        "23df2d701319a6d3c1c6be6b92c3b01e244e9136ea171e10179ab818beadadf53755b900c"
        "4decdfb742b0e00484a21b7954ba6cca95302a0b1ec623fdb9ffd93b7c599d7e39a504de7"
        "9394345ef271f55797129dfa19878f6f15c57bfbc6ee8a6cd6d3dbb874b833e1a757f01be"
        "2273f31d8dd8f2591334617bee9b2674a0a421e3171f68a958b14290f5f1dc943cbffecb7"
        "108c71e5912b718ed7cd6852d923957e7a0fa32554588872b4a1ae3ce59c50dbb27b283a2"
        "6a7472e96b54406e2969864f70d494b9866c6785f6612f6fe7e25edcb4390bb7c235f452e"
        "50438fad01f18befdac52fe1a8abca67523f989d0d339464cef18d1a05827ca888af15c2c"
        "d669c6a5d5ffab685fe10d44f7c4b4bb14279830395db88b6787b0b44cebfaa63c03f717e"
        "5ed4a06589f1ae4410378fd2194333cac3cb4f9f09e95f6ceab6ec29c61b0a250ce426b01"
        "216fe184483f1d8819b790bc285f627fd6fade74922108942d9403aaf53d0cf6227ccb560"
        "58f92b42295faedb3205b51bb4fc9f332a9eeafa2018a59048262841cb1e02acdd3033249"
        "4ec9c56fa04b32c61547bf2f61fb4b8999c4ef7ecb12477aafee76f3b1d58ef8528bb7b04"
        "7c88f81dbd63cdaf1b4e42ecd31e2b67f82bcb6d734cf39949036aa31cf49179f59c47914"
        "03f0b7d182260c0c5fb76e083a606bc85197e203a9a5e97cf30e280f557e164e4f7f587a0"
        "97dcbd7bce1e7fdbfbf03e3d3659f77a8793084955b44206218e3fb274d3f63a157d8cfc8"
        "06c6e8794519ca28ccc489130d19f934c50e7af6215cab09cedf16f040ad550f7a8d20fd7"
        "f17ebd011e3805ffe004b4fefe9679823face8588aa1c5cd4c3f801d1ad6fc2e988a947e9"
        "9f1605a87deb4520677eae9d48e6291f32ec6d60b7393d90a9fd5000d6b32ec839b29ab8f"
        "d59c2fafb38cff9c17252d71bffa880e199112bf5822b519c79c31255de959c192737f427"
        "2e72d5ef039164a7ce84b1fd883b282276cb58447dc37c76027cce3bd412907db81d9e4c0"
        "a632c68e1888045870a09b3439671692f8e4b1cc6b6cbdfe0f154617e46df430746b2f1d1"
        "2a5864260c452a814359651fb222ac83ea119fbe42b474d984f57e8aca7cb505f0c6d3e5b"
        "06eeab8286ce2bead87b7c26d3bd5fc85351a623e9d58f56d0e450862381f36a4eb9640dc"
        "384c9cfeeed11ad6a72d0c375ae4a0fa135cd78cdc0450f548a0a9484f9fc3c5281d2b14b"
        "c6af5bce00f6de79a460e4e1414c1c86a75683064f2ae290f79b979c8def99d94e7db7672"
        "f7b20477c112810bfb149e3e3ab68a099fc5a5afb67a7096fc88e7fcfa4499ec70492c77e"
        "84659578a708ccbb6d498c302807cb4d8bf302f10498258f4c99d98f3c3ae2f1e222da34d"
        "4602976c4ab31dc55eec9342d04edd94bbfb3d79b308150c8227e1f52e846bae059e25dd7"
        "18f7652b193dfa766033f0470c12efbc95ffd25352844efd3e41d474fbdfb8cf174692548"
        "f"
    };

    const std::vector<std::string> SHA_256_TEST_RESULTS = {
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        "28969cdfa74a12c82f3bad960b0b000aca2ac329deea5c2328ebc6f2ba9802c1",
        "5ca7133fa735326081558ac312c620eeca9970d1e70a4b95533d956f072d1f98",
        "42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa",
        "888e223d5a497fc679c3ecfe98bf7dc531a4adf3ccf0e6d586c8912ebf781af1"
    };

    const std::vector<std::string> SHA_512_TEST_VECTORS = {
        "",
        "21",
        "9083",
        "fd2203e467574e834ab07c9097ae164532f24be1eb5d88f1af7748ceff0d2c67a21f4e409"
        "7f9d3bb4e9fbf97186e0db6db0100230a52b453d421f8ab9c9a6043aa3295ea20d2f06a2f"
        "37470d8a99075f1b8a8336f6228cf08b5942fc1fb4299c7d2480e8e82bce175540bdfad77"
        "52bc95b577f229515394f3ae5cec870a4b2f8",
        "eb22f73a99b3b00d6c9b15c8be5cf11a5435d561e398bbc1d3dee973423a18604cebd6ef8"
        "ab6291bd89475c85ca89f57c4737378ce4b433f0e09d17bec037043ec3a65d132d4939445"
        "cc588013e8b2a935eba610750ce2bc8da8a10d3234cf4e51756d8999f3389f39ab8597eb0"
        "47037310435d1dec4de60d9211fd41eb57ea90506b6bec7fc5e653e99c6b6ece4222a3f26"
        "ea651359805279de1475b76ec57d90530819c5d84e0ddbfee1851f700fa9e8b98b711cb51"
        "0bbf545479b0547ebf278c62b429a6d50c099429fab8b9181ddb225e1a0ffbed6c2d6c082"
        "ec11293a2f47c5e6e04ea5aba6fecdc8d1382dbeed27d3b497c65e1990a3b68b10e2faf85"
        "d84ba039bea67a9e69407249e2f2d0f02cf29cc01797c234ce48f2bb080a46ea3b80c481d"
        "749a90a9e3597abb777abc443190f0841e003a1f22b22a74f40de42a6a237e0bf728a9167"
        "22994cf18fa0ddad571eac8bfd1a731e3895003451fa11922676d1078f83c56c9e038831b"
        "974aed4332807663d9b2a9b27b65067bc0ef996940dcd201f2919070e72f7f8738baa4043"
        "d7b3613ffa2b014f030249fda14d2c51694f999feefedf0a4416a64c5a6289ff8a2afd62b"
        "c16d93507a06b85459ebc1515e2e4904a095cf8135edbb2556a09ccdbf8ab4455509b6040"
        "b53ce16060de259001918eceaaf98e5d5af934212358423986eb536a7ab0572abb4ea0863"
        "5fb29642610e9e1663d038e759f403220fdd61f704e2e4ff7c090322d11212b80ab871eea"
        "e33036bdb01e13492e650cd5532f1a2f44238a5a4491f3751511c4c357c50caaa9001333c"
        "b24a840372115d30083a61e5dd1ae04a4e2fb4ddac0e6809879ca0fb2c81b4b16cda9f539"
        "f2e18c7e9a4fe80bc73f9969e995f1c2dd054ae60be36490f6c385fb6f94c2b769047435c"
        "973f6ce62938b1c3c700b7db4a584b8f1c543d42a80571180f5533fd1d31cbea7bee51632"
        "6e5c3451f6155be65bc089d20c47ecc1f630d1a8480123ebe208819b8d1d93c5048f3aa0b"
        "499fdf07c70e11314db5b5b79c5c45c19671415c5aa174b6b1aa7891a96a7cf27f3c4519c"
        "28573b6b68a065af785202c5ef70cdd283f80de556c1fd04f50423cccd5f884f5954c19c8"
        "a4c6efbbb0342ea24da92bd28ac79687c83af583cc8550f812012ea86422c14bacfd5e310"
        "70730281a202ca0a9d0470d65d0e78d06cc98cb9a6701231d80160749b38fa1af2398b907"
        "77cadac5235ab0ede990b5279f7b0d1632e4ade6eb80c324f16952d310cf1dbcb8ebc7054"
        "93538058dd743578661852d6bbe90e7ed6749e5bed68e99543e1ce0eb79b092c9426d57c5"
        "a47ebd0ab2fc881979124a0d335f3323be9b29794c7e1b165bdd932be8178b98dfaf82b2d"
        "73acf44fc7c34d44cceb62707c2ac07600a1743799631f88efd490f711ecd073607a9f42f"
        "16cfc81b5d17d29a4b4a84e4897979bb2394b79ee838314013c790382bbe32fa27437306d"
        "6fe2c5f7e2a7a3fc4ecc109cbe3f07fa0000635d698d5456db70545f9b85d76ff123e3ea3"
        "7570d98016224bd8472b99ed4a9638ae675ec6963c76f8d2550cd4a2fc0f200152ef1a889"
        "fa46990e7f15497beea32e55cb4216ef48cfbfb420a868564a7ebe1ea7e6aa3304f4b3cec"
        "c91865030909c1a15ec0bd884f36bca8dff671f00f898b65a53198712cda237d3c4d20c2f"
        "ba78b5be4094b9fc7925caf118a2d4b4ee047eaecd7300942fc8fc3d15b90a2c9f28ab5cf"
        "66e0e2fdf5257e2aac831b6cda5da1dcc433a1999ac6e510aa7d4c7ad408d5a53e43f9f84"
        "95d3eb2c33909725e14b3fc6069e03a9d110c81a2af6716344be3f1f93d8bc0744d162f37"
        "18bc"
    };

    const std::vector<std::string> SHA_512_TEST_RESULTS = {
        "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5"
        "d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e",
        "3831a6a6155e509dee59a7f451eb35324d8f8f2df6e3708894740f98fdee23889f4de5adb"
        "0c5010dfb555cda77c8ab5dc902094c52de3278f35a75ebc25f093a",
        "55586ebba48768aeb323655ab6f4298fc9f670964fc2e5f2731e34dfa4b0c09e6e1e12e3d"
        "7286b3145c61c2047fb1a2a1297f36da64160b31fa4c8c2cddd2fb4",
        "a21b1077d52b27ac545af63b32746c6e3c51cb0cb9f281eb9f3580a6d4996d5c9917d2a6e"
        "484627a9d5a06fa1b25327a9d710e027387fc3e07d7c4d14c6086cc",
        "97c433de3cd8f18b75e494bafc3c813e112b2261f1261ea6fab9dce0aaee5e5359f0e763f"
        "02154cac1589bb70c2cbe9cd22f976559032923c4e2beaf3f88f196"
    };
};

K_TEST_F(HasherTest, sha256)
{
    ASSERT_THAT(SHA_256_TEST_VECTORS.size(), Eq(SHA_256_TEST_RESULTS.size()));

    for (unsigned int i = 0; i < SHA_256_TEST_VECTORS.size(); ++i)
    {
        auto in = Util::Hex::decode(SHA_256_TEST_VECTORS[i]);
        auto expectedOutHex = SHA_256_TEST_RESULTS[i];
        auto expectedOutRaw = Util::Hex::decode(SHA_256_TEST_RESULTS[i]);

        SCOPED_TRACE(i);
        EXPECT_THAT(Crypto::Hasher::sha256Hex(in), Eq(expectedOutHex));
        EXPECT_THAT(Crypto::Hasher::sha256(in), Eq(expectedOutRaw));
    }
}

K_TEST_F(HasherTest, sha512)
{
    ASSERT_THAT(SHA_512_TEST_VECTORS.size(), Eq(SHA_512_TEST_RESULTS.size()));

    for (unsigned int i = 0; i < SHA_512_TEST_VECTORS.size(); ++i)
    {
        auto inData = Util::Hex::decode(SHA_512_TEST_VECTORS[i]);
        std::istringstream inStream(Util::to_string(inData));
        auto expectedOutHex = SHA_512_TEST_RESULTS[i];
        auto expectedOutRaw = Util::Hex::decode(SHA_512_TEST_RESULTS[i]);

        SCOPED_TRACE(i);
        EXPECT_THAT(Crypto::Hasher::sha512(inData), Eq(expectedOutRaw));
        EXPECT_THAT(Crypto::Hasher::sha512Hex(inData), Eq(expectedOutHex));
        EXPECT_THAT(Crypto::Hasher::sha512Hex(inStream), Eq(expectedOutHex));
    }
}

K_TEST_F(HasherTest, eightByteHash)
{
    auto in = Util::Hex::decode(SHA_256_TEST_VECTORS[0]);
    EXPECT_THAT(Crypto::Hasher::eightByteHash(in), Eq(-2039914840885289964));
}
