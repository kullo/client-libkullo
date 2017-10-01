// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from datetime.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api_impl/DateTime.h"
#include <kulloclient/nn.h>

namespace JNI { namespace Kullo { namespace Api {

class DateTime final {
public:
    using CppType = ::Kullo::Api::DateTime;
    using JniType = jobject;

    using Boxed = DateTime;

    ~DateTime();

    static CppType toCpp(JNIEnv* jniEnv, JniType j);
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c);

private:
    DateTime();
    friend ::djinni::JniClass<DateTime>;

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("net/kullo/libkullo/api/DateTime") };
    const jmethodID jconstructor { ::djinni::jniGetMethodID(clazz.get(), "<init>", "(SBBBBBS)V") };
    const jfieldID field_year { ::djinni::jniGetFieldID(clazz.get(), "year", "S") };
    const jfieldID field_month { ::djinni::jniGetFieldID(clazz.get(), "month", "B") };
    const jfieldID field_day { ::djinni::jniGetFieldID(clazz.get(), "day", "B") };
    const jfieldID field_hour { ::djinni::jniGetFieldID(clazz.get(), "hour", "B") };
    const jfieldID field_minute { ::djinni::jniGetFieldID(clazz.get(), "minute", "B") };
    const jfieldID field_second { ::djinni::jniGetFieldID(clazz.get(), "second", "B") };
    const jfieldID field_tzOffsetMinutes { ::djinni::jniGetFieldID(clazz.get(), "tzOffsetMinutes", "S") };
};

} } }  // namespace JNI::Kullo::Api
