/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/crypto/pbkdf.h"

#include "argon2/argon2.h"

namespace Kullo {
namespace Crypto {

std::vector<unsigned char> PBKDF::argon2i(
        uint32_t times, uint32_t memoryKiB, uint32_t parallelism,
        const std::string &password,
        const std::vector<unsigned char> &salt,
        size_t outputBytes)
{
    std::vector<unsigned char> output(outputBytes);

    argon2_context ctx;
    ctx.out = output.data();
    ctx.outlen = static_cast<uint32_t>(output.size());
    ctx.pwd = reinterpret_cast<unsigned char *>(
                const_cast<char *>(password.c_str()));
    ctx.pwdlen = static_cast<uint32_t>(password.size());
    ctx.salt = const_cast<unsigned char *>(salt.data());
    ctx.saltlen = static_cast<uint32_t>(salt.size());
    ctx.secret = nullptr;
    ctx.secretlen = 0;
    ctx.ad = nullptr;
    ctx.adlen = 0;
    ctx.t_cost = times;
    ctx.m_cost = memoryKiB;
    ctx.lanes = parallelism;
    ctx.threads = parallelism;
    ctx.version = ARGON2_VERSION_13;
    ctx.allocate_cbk = nullptr;
    ctx.free_cbk = nullptr;
    ctx.flags = ARGON2_DEFAULT_FLAGS;

    auto result = argon2i_ctx(&ctx);
    if (result != ARGON2_OK)
    {
        auto errmsg = argon2_error_message(result);
        throw PBKDFFailed(
                    std::string("argon2i_ctx failed: ") +
                    std::string(errmsg));
    }
    return output;
}

}
}
