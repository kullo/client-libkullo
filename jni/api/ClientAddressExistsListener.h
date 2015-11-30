// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from api.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/ClientAddressExistsListener.h"

namespace JNI { namespace Kullo { namespace Api {

class ClientAddressExistsListener final : ::djinni::JniInterface<::Kullo::Api::ClientAddressExistsListener, ClientAddressExistsListener> {
public:
    using CppType = std::shared_ptr<::Kullo::Api::ClientAddressExistsListener>;
    using JniType = jobject;

    using Boxed = ClientAddressExistsListener;

    ~ClientAddressExistsListener();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<ClientAddressExistsListener>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return {jniEnv, ::djinni::JniClass<ClientAddressExistsListener>::get()._toJava(jniEnv, c)}; }

private:
    ClientAddressExistsListener();
    friend ::djinni::JniClass<ClientAddressExistsListener>;
    friend ::djinni::JniInterface<::Kullo::Api::ClientAddressExistsListener, ClientAddressExistsListener>;

    class JavaProxy final : ::djinni::JavaProxyCacheEntry, public ::Kullo::Api::ClientAddressExistsListener
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        void finished(const std::shared_ptr<::Kullo::Api::Address> & address, bool exists) override;
        void error(const std::shared_ptr<::Kullo::Api::Address> & address, ::Kullo::Api::NetworkError error) override;

    private:
        using ::djinni::JavaProxyCacheEntry::getGlobalRef;
        friend ::djinni::JniInterface<::Kullo::Api::ClientAddressExistsListener, ::JNI::Kullo::Api::ClientAddressExistsListener>;
        friend ::djinni::JavaProxyCache<JavaProxy>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("net/kullo/libkullo/api/ClientAddressExistsListener") };
    const jmethodID method_finished { ::djinni::jniGetMethodID(clazz.get(), "finished", "(Lnet/kullo/libkullo/api/Address;Z)V") };
    const jmethodID method_error { ::djinni::jniGetMethodID(clazz.get(), "error", "(Lnet/kullo/libkullo/api/Address;Lnet/kullo/libkullo/api/NetworkError;)V") };
};

} } }  // namespace JNI::Kullo::Api
