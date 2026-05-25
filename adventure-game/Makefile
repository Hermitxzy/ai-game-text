# Adventure Game Makefile
# 支持 Linux、macOS 和 Android arm64 编译

# 编译器和工具
CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -ldl

# 平台检测
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Android 检测
ifeq ($(ANDROID),1)
    # Android 编译配置
    ifeq ($(ARCH),arm64)
        CC = aarch64-linux-android21-clang
        CFLAGS += -fPIC -D__ANDROID__
        LDFLAGS += -landroid -llog
    else ifeq ($(ARCH),arm)
        CC = armv7a-linux-androideabi21-clang
        CFLAGS += -fPIC -D__ANDROID__
        LDFLAGS += -landroid -llog
    endif
    LIB_DIR = libs/android/$(ARCH)
else ifeq ($(UNAME_S),Darwin)
    # macOS
    CFLAGS += -D_DARWIN_C_SOURCE
    LDFLAGS += -Wl,-rpath,./libs/macos
    LIB_DIR = libs/macos
else
    # Linux
    CFLAGS += -D_GNU_SOURCE
    LDFLAGS += -Wl,-rpath,./libs/linux -Wl,-rpath,/usr/local/lib
    LIB_DIR = libs/linux
endif

# 目录结构
SRC_DIR = src
INC_DIR = include
OBJ_DIR = build
BIN_DIR = bin

# 源代码文件
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/game.c \
       $(SRC_DIR)/scene.c \
       $(SRC_DIR)/inventory.c \
       $(SRC_DIR)/npc.c \
       $(SRC_DIR)/quest.c \
       $(SRC_DIR)/savegame.c \
       $(SRC_DIR)/ai.c

OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = $(BIN_DIR)/adventure-game

# 默认目标
all: directories $(TARGET)

# 创建目录
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR) data/saves models

# 编译目标
$(TARGET): $(OBJS)
	@echo "链接: $@"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "编译完成：$@"
	@echo ""
	@echo "运行方式:"
	@echo "  ./$(TARGET)                    # 预设回复模式"
	@echo "  ./$(TARGET) models/xxx.gguf    # 加载 GGUF 模型"

# 编译对象
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "编译：$<"
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# 清理
clean:
	@echo "清理编译文件..."
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "清理完成"

# 深度清理
distclean: clean
	rm -rf data/saves/*.dat
	@echo "清理完成"

# 安装 llama.cpp（如果未安装）
install-llama:
	@echo "正在安装 llama.cpp..."
	@echo "请参考：https://github.com/ggerganov/llama.cpp"
	@echo ""
	@echo "快速安装 (Linux):"
	@echo "  git clone https://github.com/ggerganov/llama.cpp"
	@echo "  cd llama.cpp && make -j"
	@echo "  sudo make install"
	@echo ""
	@echo "Android 编译:"
	@echo "  使用 NDK 交叉编译 libllama.so"
	@echo "  参考 llama.cpp 的 Android 构建文档"

# Android 编译
android-arm64:
	@echo "为 Android arm64 编译..."
	$(MAKE) ANDROID=1 ARCH=arm64 all
	@echo "输出：$(TARGET) (Android arm64)"

android-arm:
	@echo "为 Android armv7 编译..."
	$(MAKE) ANDROID=1 ARCH=arm all
	@echo "输出：$(TARGET) (Android arm)"

# 运行测试
run: all
	@echo "启动游戏（预设回复模式）..."
	./$(TARGET)

# 帮助
help:
	@echo "Adventure Game 编译系统"
	@echo ""
	@echo "make          - 编译游戏（当前平台）"
	@echo "make clean    - 清理编译文件"
	@echo "make run      - 编译并运行"
	@echo "make android-arm64  - 编译 Android arm64 版本"
	@echo "make android-arm    - 编译 Android armv7 版本"
	@echo "make install-llama  - 显示 llama.cpp 安装说明"
	@echo "make help     - 显示此帮助"

.PHONY: all clean distclean directories install-llama android-arm64 android-arm run help
