#ifndef AI_H
#define AI_H

#include "types.h"

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "AdventureGame"
#define LOGI(...) printf(__VA_ARGS__)
#define LOGE(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOGI(...) printf(__VA_ARGS__)
#define LOGE(...) fprintf(stderr, __VA_ARGS__)
#endif

// llama.cpp 前向声明
typedef struct llama_model llama_model;
typedef struct llama_context llama_context;
typedef int32_t llama_token;

// AI 模块函数声明
int ai_init(AIContext *ai, const char *model_path);
char* ai_generate_response(AIContext *ai, const char *prompt, int max_tokens);
void ai_cleanup(AIContext *ai);
const char* ai_get_error(void);
bool ai_is_loaded(AIContext *ai);

#endif
