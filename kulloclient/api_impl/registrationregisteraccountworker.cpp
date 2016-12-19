/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/registrationregisteraccountworker.h"

#include "kulloclient/api/Address.h"
#include "kulloclient/api/AddressNotAvailableReason.h"
#include "kulloclient/api_impl/challengeimpl.h"
#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/api_impl/masterkeyimpl.h"
#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/crypto/symmetriccryptor.h"
#include "kulloclient/crypto/symmetrickeygenerator.h"
#include "kulloclient/crypto/symmetrickeyloader.h"
#include "kulloclient/dao/asymmetrickeypairdao.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/registry.h"

namespace Kullo {
namespace ApiImpl {

RegistrationRegisterAccountWorker::RegistrationRegisterAccountWorker(
        std::shared_ptr<Api::Address> address,
        Crypto::SymmetricKey masterKey,
        Crypto::SymmetricKey privateDataKey,
        Crypto::PrivateKey keypairEncryption,
        Crypto::PrivateKey keypairSignature,
        const std::string &acceptedTerms,
        boost::optional<Protocol::Challenge> challenge,
        const std::string &challengeAnswer,
        std::shared_ptr<Api::RegistrationRegisterAccountListener> listener)
    : address_(address)
    , masterKey_(masterKey)
    , privateDataKey_(privateDataKey)
    , keypairEncryption_(keypairEncryption)
    , keypairSignature_(keypairSignature)
    , acceptedTerms_(acceptedTerms)
    , challenge_(challenge)
    , challengeAnswer_(challengeAnswer)
    , accountsClient_(Registry::httpClientFactory()->createHttpClient())
    , listener_(listener)
{}

void RegistrationRegisterAccountWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("RegAccW");

    boost::optional<std::string> optionalChallengeAnswer;
    if (!challengeAnswer_.empty())
    {
        optionalChallengeAnswer.emplace(challengeAnswer_);
    }

    try
    {
        auto challenge = accountsClient_.registerAccount(
                    Util::KulloAddress(address_->toString()),
                    encodeSymmKeys(),
                    encodeKeyPair(keypairEncryption_),
                    encodeKeyPair(keypairSignature_),
                    acceptedTerms_,
                    challenge_,
                    optionalChallengeAnswer);
        auto masterKey = std::make_shared<MasterKeyImpl>(masterKey_.toVector());

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            if (challenge)
            {
                auto apiChallenge =  std::make_shared<ChallengeImpl>(*challenge);
                listener_->challengeNeeded(address_, apiChallenge);
            }
            else
            {
                listener_->finished(address_, masterKey);
            }
        }
    }
    catch (Protocol::AddressBlocked)
    {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->addressNotAvailable(
                        address_,
                        Api::AddressNotAvailableReason::Blocked);
        }
    }
    catch (Protocol::AddressExists)
    {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->addressNotAvailable(
                        address_,
                        Api::AddressNotAvailableReason::Exists);
        }
    }
    catch (std::exception &ex)
    {
        Log.e() << "RegistrationRegisterAccountWorker failed: "
                << Util::formatException(ex);

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->error(
                        address_,
                        toNetworkError(std::current_exception()));
        }
    }
}

void RegistrationRegisterAccountWorker::cancel()
{
    // thread-safe, can be called without locking
    accountsClient_.cancel();

    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    listener_.reset();
}

Protocol::SymmetricKeys RegistrationRegisterAccountWorker::encodeSymmKeys()
{
    std::vector<unsigned char> privateDataKeyIV =
            Crypto::SymmetricKeyGenerator().makeRandomIV(
                Crypto::SymmetricCryptor::RANDOM_IV_BYTES);

    Protocol::SymmetricKeys result;
    result.loginKey = Crypto::SymmetricKeyGenerator().makeLoginKey(
                Util::KulloAddress(address_->toString()),
                Util::MasterKey(masterKey_.toVector())).toHex();
    result.privateDataKey = Crypto::SymmetricCryptor().encrypt(
                privateDataKey_.toVector(),
                Crypto::SymmetricKeyLoader::fromMasterKey(
                    Util::MasterKey(masterKey_.toVector())),
                privateDataKeyIV,
                Crypto::PrependIV::True);
    return result;
}

Protocol::KeyPair RegistrationRegisterAccountWorker::encodeKeyPair(
        const Crypto::PrivateKey &privKey)
{
    static const Util::DateTime validFrom  =
            Util::DateTime::nowUtc() - std::chrono::hours(24);
    static const Util::DateTime validUntil =
            Util::DateTime(2024, 1, 1, 0, 0, 0);

    std::vector<unsigned char> privkeyIV =
            Crypto::SymmetricKeyGenerator().makeRandomIV(
                Crypto::SymmetricCryptor::RANDOM_IV_BYTES);

    Protocol::KeyPair result{validFrom, validUntil};
    result.type = Crypto::toString(privKey.type());
    result.pubkey = Crypto::AsymmetricKeyLoader().toVector(privKey.pubkey());
    result.privkey = Crypto::SymmetricCryptor().encrypt(
                privKey.toVector(),
                privateDataKey_,
                privkeyIV,
                Crypto::PrependIV::True);
    return result;
}

}
}
