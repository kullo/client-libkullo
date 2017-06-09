// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from messages.djinni

#include "MessagesSearchResult.h"  // my header
#include "DateTime.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

MessagesSearchResult::MessagesSearchResult() = default;

MessagesSearchResult::~MessagesSearchResult() = default;

auto MessagesSearchResult::fromCpp(JNIEnv* jniEnv, const CppType& c) -> ::djinni::LocalRef<JniType> {
    const auto& data = ::djinni::JniClass<MessagesSearchResult>::get();
    auto r = ::djinni::LocalRef<JniType>{jniEnv->NewObject(data.clazz.get(), data.jconstructor,
                                                           ::djinni::get(::djinni::I64::fromCpp(jniEnv, c.msgId)),
                                                           ::djinni::get(::djinni::I64::fromCpp(jniEnv, c.convId)),
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.senderAddress)),
                                                           ::djinni::get(::JNI::Kullo::Api::DateTime::fromCpp(jniEnv, c.dateReceived)),
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.snippet)),
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.boundary)))};
    ::djinni::jniExceptionCheck(jniEnv);
    return r;
}

auto MessagesSearchResult::toCpp(JNIEnv* jniEnv, JniType j) -> CppType {
    ::djinni::JniLocalScope jscope(jniEnv, 7);
    assert(j != nullptr);
    const auto& data = ::djinni::JniClass<MessagesSearchResult>::get();
    return {::djinni::I64::toCpp(jniEnv, jniEnv->GetLongField(j, data.field_msgId)),
            ::djinni::I64::toCpp(jniEnv, jniEnv->GetLongField(j, data.field_convId)),
            ::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_senderAddress)),
            ::JNI::Kullo::Api::DateTime::toCpp(jniEnv, jniEnv->GetObjectField(j, data.field_dateReceived)),
            ::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_snippet)),
            ::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_boundary))};
}

} } }  // namespace JNI::Kullo::Api