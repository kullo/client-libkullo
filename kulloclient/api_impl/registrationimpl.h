/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/optional.hpp>

#include "kulloclient/api/Registration.h"
#include "kulloclient/crypto/privatekey.h"
#include "kulloclient/crypto/symmetrickey.h"
#include "kulloclient/protocol/httpstructs.h"

namespace Kullo {
namespace ApiImpl {

class RegistrationImpl : public Api::Registration
{
public:
    RegistrationImpl(
            std::unique_ptr<Crypto::SymmetricKey> masterKey,
            std::unique_ptr<Crypto::SymmetricKey> privateDataKey,
            std::unique_ptr<Crypto::PrivateKey> keypairEncryption,
            std::unique_ptr<Crypto::PrivateKey> keypairSignature);

    nn_shared_ptr<Api::AsyncTask> registerAccountAsync(
            const Api::Address &address,
            const boost::optional<std::string> &acceptedTerms,
            const std::shared_ptr<Api::Challenge> &challenge,
            const std::string &challengeAnswer,
            const nn_shared_ptr<Api::RegistrationRegisterAccountListener> &listener
            ) override;

private:
    boost::optional<Protocol::Challenge> toHttpChallenge(
            std::shared_ptr<Api::Challenge> challenge);

    std::unique_ptr<Crypto::SymmetricKey> masterKey_;
    std::unique_ptr<Crypto::SymmetricKey> privateDataKey_;
    std::unique_ptr<Crypto::PrivateKey> keypairEncryption_;
    std::unique_ptr<Crypto::PrivateKey> keypairSignature_;
};

}
}
