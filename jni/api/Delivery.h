// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from api.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/Delivery.h"

namespace JNI { namespace Kullo { namespace Api {

class Delivery final : ::djinni::JniInterface<::Kullo::Api::Delivery, Delivery> {
public:
    using CppType = std::shared_ptr<::Kullo::Api::Delivery>;
    using JniType = jobject;

    using Boxed = Delivery;

    ~Delivery();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<Delivery>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return {jniEnv, ::djinni::JniClass<Delivery>::get()._toJava(jniEnv, c)}; }

private:
    Delivery();
    friend ::djinni::JniClass<Delivery>;
    friend ::djinni::JniInterface<::Kullo::Api::Delivery, Delivery>;

};

} } }  // namespace JNI::Kullo::Api
