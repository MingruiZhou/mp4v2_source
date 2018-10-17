#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring

JNICALL
Java_google_mp4v2_1source_1github_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
