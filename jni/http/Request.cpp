// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http.djinni

#include "Request.h"  // my header
#include "HttpHeader.h"
#include "HttpMethod.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Http {

Request::Request() = default;

Request::~Request() = default;

auto Request::fromCpp(JNIEnv* jniEnv, const CppType& c) -> ::djinni::LocalRef<JniType> {
    const auto& data = ::djinni::JniClass<Request>::get();
    auto r = ::djinni::LocalRef<JniType>{jniEnv->NewObject(data.clazz.get(), data.jconstructor,
                                                           ::djinni::get(::JNI::Kullo::Http::HttpMethod::fromCpp(jniEnv, c.method)),
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.url)),
                                                           ::djinni::get(::djinni::List<::JNI::Kullo::Http::HttpHeader>::fromCpp(jniEnv, c.headers)))};
    ::djinni::jniExceptionCheck(jniEnv);
    return r;
}

auto Request::toCpp(JNIEnv* jniEnv, JniType j) -> CppType {
    ::djinni::JniLocalScope jscope(jniEnv, 4);
    assert(j != nullptr);
    const auto& data = ::djinni::JniClass<Request>::get();
    return {::JNI::Kullo::Http::HttpMethod::toCpp(jniEnv, jniEnv->GetObjectField(j, data.field_method)),
            ::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_url)),
            ::djinni::List<::JNI::Kullo::Http::HttpHeader>::toCpp(jniEnv, jniEnv->GetObjectField(j, data.field_headers))};
}

} } }  // namespace JNI::Kullo::Http
