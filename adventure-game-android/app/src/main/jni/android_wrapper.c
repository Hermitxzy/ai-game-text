// Android JNI 包装层
// 重定向 printf 输出到 Java

#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <android/log.h>

#define LOG_TAG "AdventureGame"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 游戏输出回调
static jobject g_activity_ref = NULL;
static jclass g_activity_class = NULL;
static JavaVM *g_jvm = NULL;

#include "types.h"

// 声明游戏函数
extern void game_init(GameContext *game);
extern void game_start(GameContext *game, AIContext *ai);
extern void game_handle_command(GameContext *game, AIContext *ai, const char *input);
extern void game_cleanup(GameContext *game, AIContext *ai);

// 输出缓冲区
static char g_output_buffer[16384];
static size_t g_output_pos = 0;

// 设置 Java 回调引用
void set_java_callback(JNIEnv *env, jobject activity) {
    if (g_activity_ref) {
        // Delete old reference
    }
    g_activity_ref = (*env)->NewGlobalRef(env, activity);
    g_activity_class = (*env)->GetObjectClass(env, g_activity_ref);
}

// 发送输出到 Java
static void flush_output_buffer(void) {
    if (!g_jvm || !g_activity_ref || g_output_pos == 0) return;
    
    JNIEnv *env;
    if ((*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6) == JNI_OK) {
        g_output_buffer[g_output_pos] = '\0';
        jstring jtext = (*env)->NewStringUTF(env, g_output_buffer);
        jmethodID method = (*env)->GetMethodID(env, g_activity_class, 
                                                "onGameOutput", "(Ljava/lang/String;)V");
        if (method) {
            (*env)->CallVoidMethod(env, g_activity_ref, method, jtext);
        }
        (*env)->DeleteLocalRef(env, jtext);
        g_output_pos = 0;
    }
}

// 重定义的 printf
int android_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    char buffer[1024];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // 追加到输出缓冲区
    if (g_output_pos + len < sizeof(g_output_buffer) - 1) {
        memcpy(g_output_buffer + g_output_pos, buffer, len);
        g_output_pos += len;
    }
    
    // 遇到换行符时立即发送
    if (strchr(buffer, '\n') != NULL) {
        flush_output_buffer();
    }
    
    return len;
}

// 重写 printf 符号（链接器级别）
#ifdef printf
#undef printf
#endif
