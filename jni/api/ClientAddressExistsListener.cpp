// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from api.djinni

#include "ClientAddressExistsListener.h"  // my header
#include "Address.h"
#include "NetworkError.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

ClientAddressExistsListener::ClientAddressExistsListener() : ::djinni::JniInterface<::Kullo::Api::ClientAddressExistsListener, ClientAddressExistsListener>() {}

ClientAddressExistsListener::~ClientAddressExistsListener() = default;

ClientAddressExistsListener::JavaProxy::JavaProxy(JniType j) : JavaProxyCacheEntry(j) { }

ClientAddressExistsListener::JavaProxy::~JavaProxy() = default;

void ClientAddressExistsListener::JavaProxy::finished(const std::shared_ptr<::Kullo::Api::Address> & c_address, bool c_exists) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::JNI::Kullo::Api::ClientAddressExistsListener>::get();
    jniEnv->CallVoidMethod(getGlobalRef(), data.method_finished,
                           ::djinni::get(::JNI::Kullo::Api::Address::fromCpp(jniEnv, c_address)),
                           ::djinni::get(::djinni::Bool::fromCpp(jniEnv, c_exists)));
    ::djinni::jniExceptionCheck(jniEnv);
}
void ClientAddressExistsListener::JavaProxy::error(const std::shared_ptr<::Kullo::Api::Address> & c_address, ::Kullo::Api::NetworkError c_error) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::JNI::Kullo::Api::ClientAddressExistsListener>::get();
    jniEnv->CallVoidMethod(getGlobalRef(), data.method_error,
                           ::djinni::get(::JNI::Kullo::Api::Address::fromCpp(jniEnv, c_address)),
                           ::djinni::get(::JNI::Kullo::Api::NetworkError::fromCpp(jniEnv, c_error)));
    ::djinni::jniExceptionCheck(jniEnv);
}

} } }  // namespace JNI::Kullo::Api
