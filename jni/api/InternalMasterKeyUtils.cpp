// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from masterkey.djinni

#include "InternalMasterKeyUtils.h"  // my header
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

InternalMasterKeyUtils::InternalMasterKeyUtils() : ::djinni::JniInterface<::Kullo::Api::InternalMasterKeyUtils, InternalMasterKeyUtils>("net/kullo/libkullo/api/InternalMasterKeyUtils$CppProxy") {}

InternalMasterKeyUtils::~InternalMasterKeyUtils() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_InternalMasterKeyUtils_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::Kullo::Api::InternalMasterKeyUtils>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jboolean JNICALL Java_net_kullo_libkullo_api_InternalMasterKeyUtils_isValid(JNIEnv* jniEnv, jobject /*this*/, jobject j_blocks)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::Kullo::Api::InternalMasterKeyUtils::isValid(::djinni::List<::djinni::String>::toCpp(jniEnv, j_blocks));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

} } }  // namespace JNI::Kullo::Api
