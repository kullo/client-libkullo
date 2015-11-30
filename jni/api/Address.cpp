// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from api.djinni

#include "Address.h"  // my header
#include "Address.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

Address::Address() : ::djinni::JniInterface<::Kullo::Api::Address, Address>("net/kullo/libkullo/api/Address$CppProxy") {}

Address::~Address() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Address_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<djinni::CppProxyHandle<::Kullo::Api::Address>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Address_create(JNIEnv* jniEnv, jobject /*this*/, jstring j_address)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::Kullo::Api::Address::create(::djinni::String::toCpp(jniEnv, j_address));
        return ::djinni::release(::JNI::Kullo::Api::Address::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_net_kullo_libkullo_api_Address_00024CppProxy_native_1toString(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Address>::get(nativeRef);
        auto r = ref->toString();
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_net_kullo_libkullo_api_Address_00024CppProxy_native_1localPart(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Address>::get(nativeRef);
        auto r = ref->localPart();
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_net_kullo_libkullo_api_Address_00024CppProxy_native_1domainPart(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Address>::get(nativeRef);
        auto r = ref->domainPart();
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_net_kullo_libkullo_api_Address_00024CppProxy_native_1isEqualTo(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_other)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Address>::get(nativeRef);
        auto r = ref->isEqualTo(::JNI::Kullo::Api::Address::toCpp(jniEnv, j_other));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_net_kullo_libkullo_api_Address_00024CppProxy_native_1isLessThan(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_rhs)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Address>::get(nativeRef);
        auto r = ref->isLessThan(::JNI::Kullo::Api::Address::toCpp(jniEnv, j_rhs));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

} } }  // namespace JNI::Kullo::Api
