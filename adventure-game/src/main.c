#include "game.h"
#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(const char *program) {
    printf("用法：%s [GGUF 模型文件路径]\n\n", program);
    printf("说明:\n");
    printf("  不带参数运行：使用预设回复模式（不需要 llama.cpp）\n");
    printf("  带 GGUF 模型路径：加载本地 AI 模型进行智能对话\n\n");
    printf("示例:\n");
    printf("  %s                              # 预设回复模式\n", program);
    printf("  %s models/qwen2.5-1.5b.gguf    # 加载 GGUF 模型\n\n", program);
    printf("支持的模型:\n");
    printf("  - Qwen2.5 系列 GGUF 格式\n");
    printf("  - Llama 系列 GGUF 格式\n");
    printf("  - 其他 llama.cpp 支持的 GGUF 模型\n\n");
    printf("Android 编译版本:\n");
    printf("  请确保 libllama.so 在 LD_LIBRARY_PATH 中\n");
    printf("  模型文件放置在 models/ 目录\n");
}

int main(int argc, char *argv[]) {
#ifdef __ANDROID__
    printf("=== Android 版本 ===\n");
#endif

    GameContext game = {0};
    AIContext ai = {0};
    
    // 初始化 AI 上下文
    ai.loaded = false;
    ai.model_path[0] = '\0';
    ai.n_ctx = 2048;  // 默认上下文长度

    // 检查命令行参数
    if (argc > 1) {
        const char *model_path = argv[1];
        
        // 检查是否是帮助请求
        if (strcmp(model_path, "-h") == 0 || strcmp(model_path, "--help") == 0 ||
            strcmp(model_path, "-help") == 0) {
            print_usage(argv[0]);
            return 0;
        }

        printf("正在初始化 AI 模块...\n");
        printf("模型路径：%s\n\n", model_path);
        
        // 尝试初始化 AI
        if (ai_init(&ai, model_path) == 0) {
            ai.loaded = true;
            printf("AI 模块初始化成功！\n\n");
        } else {
            printf("AI 模块初始化失败：%s\n", ai_get_error());
            printf("将使用预设回复模式继续游戏。\n\n");
        }
    } else {
        printf("未指定模型文件，使用预设回复模式。\n");
        printf("提示：可以带 GGUF 模型路径启动以启用 AI 对话功能。\n\n");
        print_usage(argv[0]);
        printf("\n按回车键继续...\n");
        getchar();
    }

    // 启动游戏
    game_start(&game, &ai);
    
    // 游戏主循环
    game_loop(&game, &ai);
    
    // 清理资源
    game_cleanup(&game, &ai);

    return 0;
}
