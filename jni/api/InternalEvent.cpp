// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from api.djinni

#include "InternalEvent.h"  // my header

namespace JNI { namespace Kullo { namespace Api {

InternalEvent::InternalEvent() : ::djinni::JniInterface<::Kullo::Api::InternalEvent, InternalEvent>("net/kullo/libkullo/api/InternalEvent$CppProxy") {}

InternalEvent::~InternalEvent() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_InternalEvent_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<djinni::CppProxyHandle<::Kullo::Api::InternalEvent>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

} } }  // namespace JNI::Kullo::Api
