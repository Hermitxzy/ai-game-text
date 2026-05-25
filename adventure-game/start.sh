#!/bin/bash
# 快速启动脚本

echo "====================================="
echo "  C 语言文字冒险游戏"
echo "====================================="
echo ""

# 检查可执行文件
if [ ! -f "bin/adventure-game" ]; then
    echo "游戏未编译，正在编译..."
    make
    if [ $? -ne 0 ]; then
        echo "编译失败！"
        exit 1
    fi
fi

# 检查模型文件
MODEL=""
if [ -n "$1" ]; then
    MODEL="$1"
elif [ -f "models/qwen2.5-1.5b.gguf" ]; then
    MODEL="models/qwen2.5-1.5b.gguf"
elif [ -f "models/llama-3.2-1b.gguf" ]; then
    MODEL="models/llama-3.2-1b.gguf"
fi

echo ""
if [ -n "$MODEL" ]; then
    echo "使用模型：$MODEL"
    echo "启动游戏（AI 模式）..."
    ./bin/adventure-game "$MODEL"
else
    echo "未找到模型文件，启动预设回复模式..."
    echo "提示：下载 GGUF 模型文件到 models/ 目录可启用 AI 对话"
    echo ""
    ./bin/adventure-game
fi
