#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "Dobby/dobby.h"
#include "Il2cpp/Il2cpp.h"
#include "Il2cpp/il2cpp-class.h"
#include "Includes/Logger.h"
#include "Includes/Utils.h"
#include "Includes/obfuscate.h"
#include "Menu/Menu.h"
#include "Menu/Setup.h"
#include "Includes/Macros.h"

// 声明原函数的替身指针
void (*o_ebow)(void *instance, int32_t a);

// 编写我们的汉化拦截函数
void my_ebow(void *instance, int32_t a) {
    o_ebow(instance, 3); 
}

#define targetLibName OBFUSCATE("libil2cpp.so")

// Hook 线程
void *hack_thread(void *)
{
    LOGI(OBFUSCATE("pthread created"));
    do {
        sleep(1);
    } while (!isLibraryLoaded(targetLibName));

    LOGI(OBFUSCATE("%s has been loaded"), (const char *)targetLibName);

    Il2cpp::Init();
    Il2cpp::EnsureAttached();

    uintptr_t il2cpp_base = 0;
    char line[512];
    FILE* f = fopen("/proc/self/maps", "r");
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "libil2cpp.so")) {
                il2cpp_base = strtoull(line, NULL, 16);
                break;
            }
        }
        fclose(f);
    }

    if (il2cpp_base > 0) {
        uintptr_t target_addr = il2cpp_base + 0xad0240c;
        DobbyHook((void *)target_addr, (void *)my_ebow, (void **)&o_ebow);
        LOGD("【汉化提示】成功拦截 ebow! 地址: %p", (void *)target_addr);
    } else {
        LOGD("【汉化提示】错误：未找到 libil2cpp.so 基址！");
    }
    return nullptr;
}

// 功能列表
jobjectArray GetFeatureList(JNIEnv *env, jobject context)
{
    const char *features[] = {
        OBFUSCATE("Button_Dump Info"),
        OBFUSCATE("Toggle_Example Toggle")
    };
    int Total_Feature = (sizeof features / sizeof features[0]);
    jobjectArray ret = (jobjectArray)env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")), env->NewStringUTF(""));
    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    return ret;
}

void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jstring featName, jint value, jboolean boolean, jstring str)
{
    switch (featNum) {
        case 0: Il2cpp::Dump(env); break;
    }
}

__attribute__((constructor)) void lib_main()
{
    pthread_t ptid;
    pthread_create(&ptid, nullptr, hack_thread, nullptr);
}

// 注册 Native 方法
int RegisterMenu(JNIEnv *env) {
    JNINativeMethod methods[] = {
        {OBFUSCATE("Icon"), OBFUSCATE("()Ljava/lang/String;"), reinterpret_cast<void *>(Icon)},
        {OBFUSCATE("IconWebViewData"), OBFUSCATE("()Ljava/lang/String;"), reinterpret_cast<void *>(IconWebViewData)},
        {OBFUSCATE("IsGameLibLoaded"), OBFUSCATE("()Z"), reinterpret_cast<void *>(isGameLibLoaded)},
        {OBFUSCATE("Init"), OBFUSCATE("(Landroid/content/Context;Landroid/widget/TextView;Landroid/widget/TextView;)V"), reinterpret_cast<void *>(Init)},
        {OBFUSCATE("SettingsList"), OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(SettingsList)},
        {OBFUSCATE("GetFeatureList"), OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(GetFeatureList)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Menu"));
    return (clazz && env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) == 0) ? JNI_OK : JNI_ERR;
}

int RegisterPreferences(JNIEnv *env) {
    JNINativeMethod methods[] = {
        {OBFUSCATE("Changes"), OBFUSCATE("(Landroid/content/Context;ILjava/lang/String;IZLjava/lang/String;)V"), reinterpret_cast<void *>(Changes)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Preferences"));
    return (clazz && env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) == 0) ? JNI_OK : JNI_ERR;
}

int RegisterMain(JNIEnv *env) {
    JNINativeMethod methods[] = {
        {OBFUSCATE("CheckOverlayPermission"), OBFUSCATE("(Landroid/content/Context;)V"), reinterpret_cast<void *>(CheckOverlayPermission)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Main"));
    return (clazz && env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) == 0) ? JNI_OK : JNI_ERR;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    vm->GetEnv((void **)&env, JNI_VERSION_1_6);
    if (RegisterMenu(env) != 0 || RegisterPreferences(env) != 0 || RegisterMain(env) != 0) return JNI_ERR;
    return JNI_VERSION_1_6;
}
