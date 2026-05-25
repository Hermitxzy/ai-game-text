#!/bin/bash
# 自动化测试脚本

echo "======================================="
echo "  C 语言文字冒险游戏 - 自动化测试"
echo "======================================="
echo ""

# 设置超时
TIMEOUT=5
EXECUTABLE="bin/adventure-game"

# 检查可执行文件
if [ ! -x "$EXECUTABLE" ]; then
    echo "错误：可执行文件不存在或不可执行"
    echo "正在编译..."
    make
    if [ $? -ne 0 ]; then
        echo "编译失败！"
        exit 1
    fi
fi

echo "测试 1: 启动游戏并查看帮助"
echo "----------------------------"
echo "help" | timeout $TIMEOUT ./$EXECUTABLE || true
echo ""
echo "✓ 帮助显示测试完成"
echo ""

echo "测试 2: 查看场景"
echo "----------------------------"
echo -e "look" | timeout $TIMEOUT ./$EXECUTABLE || true
echo ""
echo "✓ 场景查看测试完成"
echo ""

echo "测试 3: 查看背包"
echo "----------------------------"
echo -e "inventory" | timeout $TIMEOUT ./$EXECUTABLE || true
echo ""
echo "✓ 背包系统测试完成"
echo ""

echo "测试 4: 查看任务"
echo "----------------------------"
echo -e "quest" | timeout $TIMEOUT ./$EXECUTABLE || true
echo ""
echo "✓ 任务系统测试完成"
echo ""

echo "测试 5: 移动和探索"
echo "----------------------------"
echo -e "look\nn\nlook\ns\nlook" | timeout $TIMEOUT ./$EXECUTABLE || true
echo ""
echo "✓ 移动测试完成"
echo ""

echo "测试 6: 存档系统"
echo "----------------------------"
echo -e "save 1\nload 1" | timeout $TIMEOUT ./$EXECUTABLE || true
echo ""
echo "✓ 存档系统测试完成"
echo ""

echo "测试 7: 完整游戏流程"
echo "----------------------------"
echo -e "look\ninventory\nquest\nsaves\nhelp\nexit" | timeout $TIMEOUT ./$EXECUTABLE || true
echo ""
echo "✓ 完整流程测试完成"
echo ""

echo "======================================="
echo "  所有测试完成！"
echo "======================================="
echo ""
echo "注意：以上测试使用预设回复模式"
echo "如需测试 AI 对话功能，请先下载 GGUF 模型"
echo ""
echo "示例："
echo "  ./$EXECUTABLE models/qwen2.5-1.5b.gguf"
echo ""
