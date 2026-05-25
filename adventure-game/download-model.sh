#!/bin/bash
# GGUF 模型下载脚本

MODEL_DIR="models"
mkdir -p "$MODEL_DIR"

echo "============================================"
echo "  GGUF 模型下载脚本"
echo "============================================"
echo ""
echo "可选模型:"
echo "1. Qwen2.5 1.5B Instruct (推荐，速度快)"
echo "2. Llama 3.2 1B Instruct (最小体积)"
echo "3. Qwen2.5 0.5B Instruct (超快响应)"
echo ""

read -p "请选择模型 (1/2/3): " choice

case $choice in
    1)
        echo ""
        echo "下载 Qwen2.5 1.5B Instruct (Q4_K_M 量化)..."
        echo "文件大小：约 1.1GB"
        wget -O "$MODEL_DIR/qwen2.5-1.5b.gguf" \
            "https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct-GGUF/resolve/main/qwen2.5-1.5b-instruct-q4_k_m.gguf"
        MODEL_NAME="qwen2.5-1.5b.gguf"
        ;;
    2)
        echo ""
        echo "下载 Llama 3.2 1B Instruct (Q4_K_M 量化)..."
        echo "文件大小：约 800MB"
        wget -O "$MODEL_DIR/llama-3.2-1b.gguf" \
            "https://huggingface.co/bartowski/Llama-3.2-1B-Instruct-GGUF/resolve/main/Llama-3.2-1B-Instruct-Q4_K_M.gguf"
        MODEL_NAME="llama-3.2-1b.gguf"
        ;;
    3)
        echo ""
        echo "下载 Qwen2.5 0.5B Instruct (Q4_K_M 量化)..."
        echo "文件大小：约 400MB"
        wget -O "$MODEL_DIR/qwen2.5-0.5b.gguf" \
            "https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF/resolve/main/qwen2.5-0.5b-instruct-q4_k_m.gguf"
        MODEL_NAME="qwen2.5-0.5b.gguf"
        ;;
    *)
        echo "无效选择"
        exit 1
        ;;
esac

if [ $? -eq 0 ]; then
    echo ""
    echo "下载完成！"
    echo "运行游戏：./start.sh $MODEL_DIR/$MODEL_NAME"
else
    echo ""
    echo "下载失败，请检查网络连接"
    exit 1
fi
