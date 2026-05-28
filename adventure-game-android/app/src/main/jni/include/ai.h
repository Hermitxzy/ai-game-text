#ifndef AI_H
#define AI_H

#include <stdio.h>

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "AdventureGame"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#define LOGI(...) printf(__VA_ARGS__)
#define LOGE(...) fprintf(stderr, __VA_ARGS__)
#endif

// AI 模块函数声明（使用 void* 避免类型依赖）
int ai_init(void* ai, const char* model_path);
char* ai_generate_response(void* ai, const char* prompt, int max_tokens);
void ai_cleanup(void* ai);
const char* ai_get_error(void);
int ai_is_loaded(void* ai);

#endif
