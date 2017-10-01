// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from client.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/ClientGenerateKeysListener.h"
#include <kulloclient/nn.h>

namespace JNI { namespace Kullo { namespace Api {

class ClientGenerateKeysListener final : ::djinni::JniInterface<::Kullo::Api::ClientGenerateKeysListener, ClientGenerateKeysListener> {
public:
    using CppType = ::Kullo::nn_shared_ptr<::Kullo::Api::ClientGenerateKeysListener>;
    using CppOptType = std::shared_ptr<::Kullo::Api::ClientGenerateKeysListener>;
    using JniType = jobject;

    using Boxed = ClientGenerateKeysListener;

    ~ClientGenerateKeysListener();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) {
        DJINNI_ASSERT_MSG(j, jniEnv, "ClientGenerateKeysListener::fromCpp requires a non-null Java object");
        return kulloForcedNn(::djinni::JniClass<ClientGenerateKeysListener>::get()._fromJava(jniEnv, j));
    };
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<ClientGenerateKeysListener>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    ClientGenerateKeysListener();
    friend ::djinni::JniClass<ClientGenerateKeysListener>;
    friend ::djinni::JniInterface<::Kullo::Api::ClientGenerateKeysListener, ClientGenerateKeysListener>;

    class JavaProxy final : ::djinni::JavaProxyHandle<JavaProxy>, public ::Kullo::Api::ClientGenerateKeysListener
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        void progress(int8_t progress) override;
        void finished(const ::Kullo::nn_shared_ptr<::Kullo::Api::Registration> & registration) override;

    private:
        friend ::djinni::JniInterface<::Kullo::Api::ClientGenerateKeysListener, ::JNI::Kullo::Api::ClientGenerateKeysListener>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("net/kullo/libkullo/api/ClientGenerateKeysListener") };
    const jmethodID method_progress { ::djinni::jniGetMethodID(clazz.get(), "progress", "(B)V") };
    const jmethodID method_finished { ::djinni::jniGetMethodID(clazz.get(), "finished", "(Lnet/kullo/libkullo/api/Registration;)V") };
};

} } }  // namespace JNI::Kullo::Api
