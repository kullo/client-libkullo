// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/http/TransferProgress.h"
#include <kulloclient/nn.h>

namespace JNI { namespace Kullo { namespace Http {

class TransferProgress final {
public:
    using CppType = ::Kullo::Http::TransferProgress;
    using JniType = jobject;

    using Boxed = TransferProgress;

    ~TransferProgress();

    static CppType toCpp(JNIEnv* jniEnv, JniType j);
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c);

private:
    TransferProgress();
    friend ::djinni::JniClass<TransferProgress>;

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("net/kullo/libkullo/http/TransferProgress") };
    const jmethodID jconstructor { ::djinni::jniGetMethodID(clazz.get(), "<init>", "(JJJJ)V") };
    const jfieldID field_uploadTransferred { ::djinni::jniGetFieldID(clazz.get(), "uploadTransferred", "J") };
    const jfieldID field_uploadTotal { ::djinni::jniGetFieldID(clazz.get(), "uploadTotal", "J") };
    const jfieldID field_downloadTransferred { ::djinni::jniGetFieldID(clazz.get(), "downloadTransferred", "J") };
    const jfieldID field_downloadTotal { ::djinni::jniGetFieldID(clazz.get(), "downloadTotal", "J") };
};

} } }  // namespace JNI::Kullo::Http
