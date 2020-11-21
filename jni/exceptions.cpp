/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/util/exceptions.h"

namespace JNI {
namespace Kullo {

class JniError : public ::Kullo::Util::BaseException
{
public:
    /// @copydoc Kullo::Util::BaseException::BaseException(const std::string&)
    JniError(const std::string &message = "") throw()
        : BaseException(message) {}
    virtual ~JniError() = default;
};

}
}

namespace {

std::string javaObjectToString(JNIEnv *env, jobject obj)
{
    jmethodID toString = env->GetMethodID(
                env->FindClass("java/lang/Object"),
                "toString",
                "()Ljava/lang/String;");
    auto jstr = static_cast<jstring>(env->CallObjectMethod(obj, toString));
    return djinni::jniUTF8FromString(env, jstr);
}

std::string formatJavaException(JNIEnv *env, jthrowable javaExc)
{
    auto what = javaObjectToString(env, javaExc);

    jmethodID getStackTrace = env->GetMethodID(
                env->FindClass("java/lang/Throwable"),
                "getStackTrace",
                "()[Ljava/lang/StackTraceElement;");
    auto stacktrace = static_cast<jobjectArray>(
                env->CallObjectMethod(javaExc, getStackTrace));

    jsize len = env->GetArrayLength(stacktrace);
    for (jsize i = 0; i < len; ++i)
    {
        auto stackTraceElement = env->GetObjectArrayElement(stacktrace, i);
        what += "\n\tat " + javaObjectToString(env, stackTraceElement);
    }

    return what;
}

}

namespace djinni {

/*
 * This is called when an exception is thrown in Java code that we called.
 * It replaces a default implementation in djinni_support.cpp.
 */
__attribute__((noreturn))
void jniThrowCppFromJavaException(JNIEnv *env, jthrowable javaException)
{
    throw JNI::Kullo::JniError(formatJavaException(env, javaException));
}

}
