# 优化后的 Makefile - 简化 Android 编译

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -D_GNU_SOURCE
LDFLAGS = -ldl

# 自动检测平台
UNAME_S := $(shell uname -s)

# Android 编译配置优化
ifeq ($(ANDROID),1)
    ifeq ($(ARCH),arm64)
        API_LEVEL = 21
        TARGET = aarch64-linux-android$(API_LEVEL)
        CC = $(NDK_HOME)/toolchains/llvm/prebuilt/$(shell uname -s | tr '[:upper:]' '[:lower:]')-x86_64/bin/$(TARGET)-clang
        CFLAGS += -fPIC -D__ANDROID__ -march=armv8-a
        LDFLAGS += -landroid -llog -static-libstdc++
        OUTPUT_BIN = bin/android/arm64/adventure-game
    else ifeq ($(ARCH),arm)
        API_LEVEL = 21
        TARGET = armv7a-linux-androideabi$(API_LEVEL)
        CC = $(NDK_HOME)/toolchains/llvm/prebuilt/$(shell uname -s | tr '[:upper:]' '[:lower:]')-x86_64/bin/$(TARGET)-clang
        CFLAGS += -fPIC -D__ANDROID__ -march=armv7-a -mthumb
        LDFLAGS += -landroid -llog -static-libstdc++
        OUTPUT_BIN = bin/android/armv7/adventure-game
    endif
endif

# macOS 配置
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -D_DARWIN_C_SOURCE
    LDFLAGS += -Wl,-rpath,@executable_path/../libs/macos
endif

# Linux 配置 (默认)
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -Wl,-rpath,'$$ORIGIN/../libs/linux' -Wl,-rpath,/usr/local/lib
endif

SRC_DIR = src
INC_DIR = include
OBJ_DIR = build
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = $(BIN_DIR)/adventure-game

.PHONY: all clean directories android-arm64 android-arm help

# 默认目标
all: directories $(TARGET)
	@echo ""
	@echo "✓ 编译完成：$(TARGET)"
	@echo "  运行：./$(TARGET)"
	@echo "  或使用模型：./$(TARGET) models/xxx.gguf"

# 创建目录
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) bin/android/arm64 bin/android/armv7 data/saves models

# 编译目标
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "链接：$@"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# 编译对象
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@echo "编译：$<"
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Android arm64
android-arm64: directories
	@echo "编译 Android ARM64..."
	$(MAKE) ANDROID=1 ARCH=arm64
	@echo ""
	@echo "✓ Android ARM64 完成"
	@echo "  输出：$(OUTPUT_BIN)"

# Android armv7
android-armv7: directories
	@echo "编译 Android ARMv7..."
	$(MAKE) ANDROID=1 ARCH=arm
	@echo ""
	@echo "✓ Android ARMv7 完成"
	@echo "  输出：$(OUTPUT_BIN)"

# 清理
clean:
	@echo "清理..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)/$(TARGET)
	@echo "✓ 清理完成"

# 深度清理
distclean: clean
	@rm -rf bin/android data/saves/*.dat

# 帮助
help:
	@echo "C 语言文字冒险游戏 - 编译选项"
	@echo ""
	@echo " make            - 编译当前平台"
	@echo " make clean      - 清理"
	@echo " make android-arm64  - 编译 Android arm64"
	@echo " make android-armv7  - 编译 Android armv7"
	@echo " make distclean  - 完全清理"
	@echo ""
	@echo " Android 编译:"
	@echo "   需要设置环境变量：export ANDROID_NDK_HOME=/path/to/ndk"
	@echo "   或者使用一键脚本：./build-android.sh"
	@echo ""
