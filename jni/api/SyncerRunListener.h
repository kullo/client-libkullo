// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from api.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/SyncerRunListener.h"

namespace JNI { namespace Kullo { namespace Api {

class SyncerRunListener final : ::djinni::JniInterface<::Kullo::Api::SyncerRunListener, SyncerRunListener> {
public:
    using CppType = std::shared_ptr<::Kullo::Api::SyncerRunListener>;
    using JniType = jobject;

    using Boxed = SyncerRunListener;

    ~SyncerRunListener();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<SyncerRunListener>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return {jniEnv, ::djinni::JniClass<SyncerRunListener>::get()._toJava(jniEnv, c)}; }

private:
    SyncerRunListener();
    friend ::djinni::JniClass<SyncerRunListener>;
    friend ::djinni::JniInterface<::Kullo::Api::SyncerRunListener, SyncerRunListener>;

    class JavaProxy final : ::djinni::JavaProxyCacheEntry, public ::Kullo::Api::SyncerRunListener
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        void draftAttachmentsTooBig(int64_t convId) override;
        void finished() override;
        void error(::Kullo::Api::NetworkError error) override;

    private:
        using ::djinni::JavaProxyCacheEntry::getGlobalRef;
        friend ::djinni::JniInterface<::Kullo::Api::SyncerRunListener, ::JNI::Kullo::Api::SyncerRunListener>;
        friend ::djinni::JavaProxyCache<JavaProxy>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("net/kullo/libkullo/api/SyncerRunListener") };
    const jmethodID method_draftAttachmentsTooBig { ::djinni::jniGetMethodID(clazz.get(), "draftAttachmentsTooBig", "(J)V") };
    const jmethodID method_finished { ::djinni::jniGetMethodID(clazz.get(), "finished", "()V") };
    const jmethodID method_error { ::djinni::jniGetMethodID(clazz.get(), "error", "(Lnet/kullo/libkullo/api/NetworkError;)V") };
};

} } }  // namespace JNI::Kullo::Api
