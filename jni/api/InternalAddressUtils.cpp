// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from address.djinni

#include "InternalAddressUtils.h"  // my header
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

InternalAddressUtils::InternalAddressUtils() : ::djinni::JniInterface<::Kullo::Api::InternalAddressUtils, InternalAddressUtils>("net/kullo/libkullo/api/InternalAddressUtils$CppProxy") {}

InternalAddressUtils::~InternalAddressUtils() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_InternalAddressUtils_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::Kullo::Api::InternalAddressUtils>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jboolean JNICALL Java_net_kullo_libkullo_api_InternalAddressUtils_isValid(JNIEnv* jniEnv, jobject /*this*/, jstring j_localPart, jstring j_domainPart)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::Kullo::Api::InternalAddressUtils::isValid(::djinni::String::toCpp(jniEnv, j_localPart),
                                                             ::djinni::String::toCpp(jniEnv, j_domainPart));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

} } }  // namespace JNI::Kullo::Api
