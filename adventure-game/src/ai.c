#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

// llama.cpp 动态库函数指针类型
typedef const char* (*llama_git_version_t)(void);
typedef void (*llama_backend_init_t)(void);
typedef void (*llama_backend_free_t)(void);

// 全局动态库句柄和函数指针
static void* g_llama_lib = NULL;
static llama_git_version_t fn_llama_git_version = NULL;
static llama_backend_init_t fn_llama_backend_init = NULL;
static llama_backend_free_t fn_llama_backend_free = NULL;

static char g_error_message[1024] = {0};

static void set_error(const char* msg) {
    strncpy(g_error_message, msg, sizeof(g_error_message) - 1);
    g_error_message[sizeof(g_error_message) - 1] = '\0';
}

int ai_init(AIContext *ai, const char *model_path) {
    if (!ai || !model_path) {
        set_error("无效参数");
        return -1;
    }

    // 加载动态库
    g_llama_lib = dlopen("libllama.so", RTLD_LAZY | RTLD_LOCAL);
    if (!g_llama_lib) {
        snprintf(g_error_message, sizeof(g_error_message), 
                 "无法加载 libllama.so: %s. 请确保 llama.cpp 已编译并安装", dlerror());
        LOGE("%s\n", g_error_message);
        return -1;
    }

    // 加载必要的函数
    fn_llama_git_version = (llama_git_version_t)dlsym(g_llama_lib, "llama_git_version");
    fn_llama_backend_init = (llama_backend_init_t)dlsym(g_llama_lib, "llama_backend_init");
    fn_llama_backend_free = (llama_backend_free_t)dlsym(g_llama_lib, "llama_backend_free");

    const char* dl_error = dlerror();
    if (dl_error) {
        snprintf(g_error_message, sizeof(g_error_message), "加载函数失败：%s", dl_error);
        dlclose(g_llama_lib);
        g_llama_lib = NULL;
        return -1;
    }

    // 初始化后端
    if (fn_llama_backend_init) {
        fn_llama_backend_init();
    }

    // 设置模型路径
    strncpy(ai->model_path, model_path, sizeof(ai->model_path) - 1);
    ai->model_path[sizeof(ai->model_path) - 1] = '\0';

    ai->n_ctx = 2048;
    ai->model = NULL;
    ai->context = NULL;

    LOGI("llama.cpp 库已加载：");
    if (fn_llama_git_version) {
        LOGI("%s\n", fn_llama_git_version());
    }
    LOGI("模型文件：%s\n", model_path);
    LOGI("提示：完整的模型加载需要在 ai.c 中实现 llama_model_load 等函数\n");

    // 注意：这里是简化实现，实际使用需要完整的 llama.cpp API 调用
    // 为了保持代码简洁，我们使用预设回复模式作为备用方案
    ai->loaded = false;  // 需要实际加载模型后才设为 true

    return 0;
}

bool ai_is_loaded(AIContext *ai) {
    return ai && ai->loaded;
}

char* ai_generate_response(AIContext *ai, const char *prompt, int max_tokens) {
    (void)max_tokens;  // 未使用的参数

    // 检查是否实际加载了模型
    if (!ai || !ai->loaded || !prompt) {
        return strdup("AI 未初始化");
    }

    // 检查是否实际加载了模型
    if (!ai->model || !ai->context) {
        return strdup("AI 模型未实际加载，使用预设回复\n\n"
                     "系统提示：模型文件不存在或加载失败。\n"
                     "请确保 GGUF 模型文件存在于正确路径。\n"
                     "\n"
                     "示例回复。\n"
                     "要继续冒险，请使用：look, go [方向], take [物品], talk [NPC], use [物品], inventory, save, load");
    }

    // 简化的文本生成逻辑
    static char response[512];
    
    // 根据提示生成不同的回复
    if (strstr(prompt, "你好") || strstr(prompt, "hello")) {
        snprintf(response, sizeof(response), 
                 "你好，勇敢的冒险者！我是这个世界的智能助手。我可以回答你的问题，或者给你一些冒险的建议。");
    } else if (strstr(prompt, "任务") || strstr(prompt, "quest")) {
        snprintf(response, sizeof(response), 
                 "在这个世界中，有许多冒险等待着你。与 NPC 交谈，探索不同的场景，收集物品，你可能会发现隐藏的任务线索。");
    } else if (strstr(prompt, "商店") || strstr(prompt, "shop") || strstr(prompt, "buy")) {
        snprintf(response, sizeof(response), 
                 "附近似乎有一些商人在出售他们的商品。使用'shop'命令打开商店界面。");
    } else if (strstr(prompt, "怎么走") || strstr(prompt, "where") || strstr(prompt, "direction")) {
        snprintf(response, sizeof(response), 
                 "要在这个世界中移动，你可以使用'go [方向]'命令。先使用'look'查看当前场景的描述。");
    } else {
        snprintf(response, sizeof(response), 
                 "我听到了：%s\n\n你可以使用以下命令：\n"
                 "- look: 查看当前场景\n"
                 "- go [方向]: 移动\n"
                 "- take [物品]: 拾取物品\n"
                 "- talk [NPC]: 与 NPC 对话\n"
                 "- inventory: 查看背包\n"
                 "- save/load: 保存/加载进度", prompt);
    }

    return strdup(response);
}

void ai_cleanup(AIContext *ai) {
    if (!ai) return;

    if (fn_llama_backend_free) {
        fn_llama_backend_free();
    }

    if (g_llama_lib) {
        dlclose(g_llama_lib);
        g_llama_lib = NULL;
    }

    ai->loaded = false;
    LOGI("AI 模块已清理\n");
}

const char* ai_get_error(void) {
    return g_error_message;
}
