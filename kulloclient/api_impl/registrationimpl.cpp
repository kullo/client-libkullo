/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/registrationimpl.h"

#include "kulloclient/api_impl/asynctaskimpl.h"
#include "kulloclient/api_impl/challengeimpl.h"
#include "kulloclient/api_impl/registrationregisteraccountworker.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

RegistrationImpl::RegistrationImpl(
        std::unique_ptr<Crypto::SymmetricKey> masterKey,
        std::unique_ptr<Crypto::SymmetricKey> privateDataKey,
        std::unique_ptr<Crypto::PrivateKey> keypairEncryption,
        std::unique_ptr<Crypto::PrivateKey> keypairSignature)
    : masterKey_(std::move(masterKey))
    , privateDataKey_(std::move(privateDataKey))
    , keypairEncryption_(std::move(keypairEncryption))
    , keypairSignature_(std::move(keypairSignature))
{}

nn_shared_ptr<Api::AsyncTask> RegistrationImpl::registerAccountAsync(
        const Api::Address &address,
        const boost::optional<std::string> &acceptedTerms,
        const std::shared_ptr<Api::Challenge> &challenge,
        const std::string &challengeAnswer,
        const nn_shared_ptr<Api::RegistrationRegisterAccountListener> &listener)
{
    kulloAssert(!acceptedTerms || !acceptedTerms->empty()); // disallow empty string

    return nn_make_shared<AsyncTaskImpl>(
        std::make_shared<RegistrationRegisterAccountWorker>(
                    address,
                    *masterKey_,
                    *privateDataKey_,
                    *keypairEncryption_,
                    *keypairSignature_,
                    acceptedTerms,
                    toHttpChallenge(challenge),
                    challengeAnswer,
                    listener));
}

boost::optional<Protocol::Challenge> RegistrationImpl::toHttpChallenge(
        std::shared_ptr<Api::Challenge> challenge)
{
    if (!challenge) return boost::optional<Protocol::Challenge>();

    auto concreteChallenge = std::dynamic_pointer_cast<ChallengeImpl>(challenge);
    kulloAssert(concreteChallenge);

    return concreteChallenge->httpChallenge();
}

}
}
