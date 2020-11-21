/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <argon2/argon2.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

K_TEST(Argon2, a2iTestVector)
{
    // official test vector from
    // https://github.com/P-H-C/phc-winner-argon2/blob/master/kats/argon2i

    std::vector<unsigned char> output(32);
    std::vector<unsigned char> password(32, 1);
    std::vector<unsigned char> salt(16, 2);
    std::vector<unsigned char> secret(8, 3);
    std::vector<unsigned char> associatedData(12, 4);

    argon2_context ctx;
    ctx.out = output.data();
    ctx.outlen = static_cast<uint32_t>(output.size());
    ctx.pwd = password.data();
    ctx.pwdlen = static_cast<uint32_t>(password.size());
    ctx.salt = salt.data();
    ctx.saltlen = static_cast<uint32_t>(salt.size());
    ctx.secret = secret.data();
    ctx.secretlen = static_cast<uint32_t>(secret.size());
    ctx.ad = associatedData.data();
    ctx.adlen = static_cast<uint32_t>(associatedData.size());
    ctx.t_cost = 3;
    ctx.m_cost = 32;
    ctx.lanes = 4;
    ctx.threads = 1;
    ctx.version = ARGON2_VERSION_13;
    ctx.allocate_cbk = nullptr;
    ctx.free_cbk = nullptr;
    ctx.flags = ARGON2_DEFAULT_FLAGS;

    std::vector<unsigned char> expected {
        0xc8, 0x14, 0xd9, 0xd1, 0xdc, 0x7f, 0x37, 0xaa,
        0x13, 0xf0, 0xd7, 0x7f, 0x24, 0x94, 0xbd, 0xa1,
        0xc8, 0xde, 0x6b, 0x01, 0x6d, 0xd3, 0x88, 0xd2,
        0x99, 0x52, 0xa4, 0xc4, 0x67, 0x2b, 0x6c, 0xe8,
    };

    ASSERT_THAT(argon2i_ctx(&ctx), Eq(ARGON2_OK));
    EXPECT_THAT(output, Eq(expected));
}
