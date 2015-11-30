// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/http/ResponseListener.h"

namespace JNI { namespace Kullo { namespace Http {

class ResponseListener final : ::djinni::JniInterface<::Kullo::Http::ResponseListener, ResponseListener> {
public:
    using CppType = std::shared_ptr<::Kullo::Http::ResponseListener>;
    using JniType = jobject;

    using Boxed = ResponseListener;

    ~ResponseListener();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<ResponseListener>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return {jniEnv, ::djinni::JniClass<ResponseListener>::get()._toJava(jniEnv, c)}; }

private:
    ResponseListener();
    friend ::djinni::JniClass<ResponseListener>;
    friend ::djinni::JniInterface<::Kullo::Http::ResponseListener, ResponseListener>;

};

} } }  // namespace JNI::Kullo::Http
