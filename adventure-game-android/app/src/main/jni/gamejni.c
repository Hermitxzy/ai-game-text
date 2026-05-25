#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <android/log.h>
#include <stdio.h>

// 前向声明 - 避免直接包含头文件导致的兼容性问题
typedef struct GameContext GameContext;
typedef struct AIContext AIContext;

// 简化类型定义
struct GameContext {
    void *current_scene;
    struct {
        void *items;
        int count;
        int gold;
    } inventory;
    void *quests;
    int quest_count;
    void *npcs;
    int npc_count;
    void *scenes;
    int scene_count;
    int running;
    char player_name[64];
    int current_npc_index;
    int in_dialogue;
    int in_shop;
};

struct AIContext {
    void *model;
    void *context;
    char model_path[512];
    long long n_ctx;
    int loaded;
};

// 游戏函数声明（来自 game.c）
void game_init(GameContext *game);
void game_start(GameContext *game, AIContext *ai);
void game_handle_command(GameContext *game, AIContext *ai, const char *input);
void game_cleanup(GameContext *game, AIContext *ai);

// 简化场景结构用于获取场景名
typedef struct {
    char id[64];
    char name[64];
    char description[512];
} SimpleScene;

#define LOG_TAG "AdventureGame"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// 游戏单例
static GameContext g_game;
static AIContext g_ai;
static int g_game_initialized = 0;
static pthread_mutex_t g_game_mutex = PTHREAD_MUTEX_INITIALIZER;

// 输出重定向 - 捕获 printf 输出
static char g_output_buffer[8192];
static size_t g_output_pos = 0;

// 简单的 printf 重定向实现
int android_print(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    // 追加到输出缓冲区
    size_t len = strlen(buffer);
    if (g_output_pos + len < sizeof(g_output_buffer) - 1) {
        strcpy(g_output_buffer + g_output_pos, buffer);
        g_output_pos += len;
    }
    
    va_end(args);
    return (int)len;
}

// 重定义 printf 为宏（简化实现）
#ifdef printf
#undef printf
#endif
#define printf android_print

// JNI 方法实现
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void)reserved;
    JNIEnv *env;
    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    LOGI("JNI 库已加载");
    return JNI_VERSION_1_6;
}

JNIEXPORT jboolean JNICALL Java_com_adventure_game_GameActivity_initGame(
    JNIEnv *env, jobject thiz, jstring modelPath) {
    
    (void)thiz;
    pthread_mutex_lock(&g_game_mutex);
    
    if (g_game_initialized) {
        pthread_mutex_unlock(&g_game_mutex);
        return JNI_TRUE;
    }
    
    LOGI("初始化游戏...");
    
    // 清零初始化
    memset(&g_game, 0, sizeof(GameContext));
    memset(&g_ai, 0, sizeof(AIContext));
    
    // 处理模型路径
    if (modelPath != NULL) {
        const char *path = (*env)->GetStringUTFChars(env, modelPath, NULL);
        strncpy(g_ai.model_path, path, sizeof(g_ai.model_path) - 1);
        LOGI("模型路径：%s", path);
        (*env)->ReleaseStringUTFChars(env, modelPath, path);
        // 注意：这里简化处理，实际需要完整的模型加载逻辑
        g_ai.loaded = 0;  // 使用预回复模式
    } else {
        LOGI("未提供模型路径，使用预回复模式");
    }
    
    // 初始化游戏
    game_init(&g_game);
    game_start(&g_game, &g_ai);
    
    g_game_initialized = 1;
    pthread_mutex_unlock(&g_game_mutex);
    
    LOGI("游戏初始化完成");
    return JNI_TRUE;
}

JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_processInput(
    JNIEnv *env, jobject thiz, jstring input) {
    
    (void)thiz;
    pthread_mutex_lock(&g_game_mutex);
    
    if (!g_game_initialized) {
        pthread_mutex_unlock(&g_game_mutex);
        return (*env)->NewStringUTF(env, "游戏未初始化");
    }
    
    // 获取输入
    const char *input_text = (*env)->GetStringUTFChars(env, input, NULL);
    LOGI("用户输入：%s", input_text);
    
    // 清空输出缓冲区
    memset(g_output_buffer, 0, sizeof(g_output_buffer));
    g_output_pos = 0;
    
    // 处理命令
    game_handle_command(&g_game, &g_ai, input_text);
    
    (*env)->ReleaseStringUTFChars(env, input, input_text);
    
    // 返回输出
    const char *result = g_output_buffer[0] != '\0' ? g_output_buffer : "";
    jstring result_str = (*env)->NewStringUTF(env, result);
    
    pthread_mutex_unlock(&g_game_mutex);
    
    return result_str;
}

JNIEXPORT void JNICALL Java_com_adventure_game_GameActivity_cleanupGame(
    JNIEnv *env, jobject thiz) {
    
    (void)env;
    (void)thiz;
    pthread_mutex_lock(&g_game_mutex);
    
    if (g_game_initialized) {
        game_cleanup(&g_game, &g_ai);
        g_game_initialized = 0;
        LOGI("游戏已清理");
    }
    
    pthread_mutex_unlock(&g_game_mutex);
}

JNIEXPORT jboolean JNICALL Java_com_adventure_game_GameActivity_isGameRunning(
    JNIEnv *env, jobject thiz) {
    
    (void)env;
    (void)thiz;
    return g_game_initialized && g_game.running ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_getCurrentScene(
    JNIEnv *env, jobject thiz) {
    
    (void)thiz;
    if (!g_game_initialized || !g_game.current_scene) {
        return NULL;
    }
    
    SimpleScene *scene = (SimpleScene *)g_game.current_scene;
    return (*env)->NewStringUTF(env, scene->name);
}

JNIEXPORT jint JNICALL Java_com_adventure_game_GameActivity_getGold(
    JNIEnv *env, jobject thiz) {
    
    (void)env;
    (void)thiz;
    if (!g_game_initialized) {
        return 0;
    }
    return g_game.inventory.gold;
}
