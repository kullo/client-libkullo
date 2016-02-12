// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from push.djinni

#include "PushToken.h"  // my header
#include "PushTokenEnvironment.h"
#include "PushTokenType.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

PushToken::PushToken() = default;

PushToken::~PushToken() = default;

auto PushToken::fromCpp(JNIEnv* jniEnv, const CppType& c) -> ::djinni::LocalRef<JniType> {
    const auto& data = ::djinni::JniClass<PushToken>::get();
    auto r = ::djinni::LocalRef<JniType>{jniEnv->NewObject(data.clazz.get(), data.jconstructor,
                                                           ::djinni::get(::JNI::Kullo::Api::PushTokenType::fromCpp(jniEnv, c.type)),
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.token)),
                                                           ::djinni::get(::JNI::Kullo::Api::PushTokenEnvironment::fromCpp(jniEnv, c.environment)))};
    ::djinni::jniExceptionCheck(jniEnv);
    return r;
}

auto PushToken::toCpp(JNIEnv* jniEnv, JniType j) -> CppType {
    ::djinni::JniLocalScope jscope(jniEnv, 4);
    assert(j != nullptr);
    const auto& data = ::djinni::JniClass<PushToken>::get();
    return {::JNI::Kullo::Api::PushTokenType::toCpp(jniEnv, jniEnv->GetObjectField(j, data.field_type)),
            ::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_token)),
            ::JNI::Kullo::Api::PushTokenEnvironment::toCpp(jniEnv, jniEnv->GetObjectField(j, data.field_environment))};
}

} } }  // namespace JNI::Kullo::Api
