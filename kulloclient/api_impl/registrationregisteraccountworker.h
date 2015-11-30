/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <boost/optional.hpp>

#include "kulloclient/api/RegistrationRegisterAccountListener.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/crypto/privatekey.h"
#include "kulloclient/crypto/symmetrickey.h"
#include "kulloclient/protocol/accountsclient.h"

namespace Kullo {
namespace ApiImpl {

class RegistrationRegisterAccountWorker : public Worker
{
public:
    RegistrationRegisterAccountWorker(
            std::shared_ptr<Api::Address> address,
            Crypto::SymmetricKey masterKey,
            Crypto::SymmetricKey privateDataKey,
            Crypto::PrivateKey keypairEncryption,
            Crypto::PrivateKey keypairSignature,
            boost::optional<Protocol::Challenge> challenge,
            const std::string &challengeAnswer,
            std::shared_ptr<Api::RegistrationRegisterAccountListener> listener);

    void work() override;
    void cancel() override;

private:
    // only to be called from work() thread
    Protocol::SymmetricKeys encodeSymmKeys();
    Protocol::KeyPair encodeKeyPair(const Crypto::PrivateKey &privKey);

    // not synchronized, non-threadsafe stuff is only used from work()
    std::shared_ptr<Api::Address> address_;
    Crypto::SymmetricKey masterKey_;
    Crypto::SymmetricKey privateDataKey_;
    Crypto::PrivateKey keypairEncryption_;
    Crypto::PrivateKey keypairSignature_;

    boost::optional<Protocol::Challenge> challenge_;
    std::string challengeAnswer_;
    Protocol::AccountsClient accountsClient_;

    // all uses must be synchronized
    std::shared_ptr<Api::RegistrationRegisterAccountListener> listener_;
};

}
}
