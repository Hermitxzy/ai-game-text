#!/bin/bash
# 从主项目同步源代码到 Android 项目

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MAIN_PROJECT="$SCRIPT_DIR/../adventure-game"
ANDROID_JNI="$SCRIPT_DIR/app/src/main/jni"

echo "同步源代码..."

# 复制源文件
for file in ai.c game.c inventory.c npc.c quest.c savegame.c scene.c; do
    cp "$MAIN_PROJECT/src/$file" "$ANDROID_JNI/"
    echo "  ✓ $file"
done

# 复制头文件
mkdir -p "$ANDROID_JNI/include"
for file in ai.h game.h inventory.h npc.h quest.h savegame.h scene.h types.h; do
    cp "$MAIN_PROJECT/include/$file" "$ANDROID_JNI/include/"
    echo "  ✓ include/$file"
done

# 创建 Android.mk
cat > "$ANDROID_JNI/Android.mk" << 'EOF'
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := adventure-game

# 源文件
LOCAL_SRC_FILES := \
    ai.c \
    game.c \
    inventory.c \
    npc.c \
    quest.c \
    savegame.c \
    scene.c \
    gamejni.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_CFLAGS := -D__ANDROID__ -Wall -Wextra -O2
LOCAL_LDLIBS := -llog -landroid

include $(BUILD_SHARED_LIBRARY)
EOF

echo ""
echo "✓ 同步完成"
echo ""
echo "下一步:"
echo "  cd $SCRIPT_DIR"
echo "  ./build-apk.sh debug"
