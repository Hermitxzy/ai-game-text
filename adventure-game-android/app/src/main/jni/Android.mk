LOCAL_PATH := $(call my-dir)

# 编译 llama.cpp 库
include $(CLEAR_VARS)
LOCAL_MODULE := llama

# llama.cpp 核心源文件
LOCAL_SRC_FILES := \
    llama.cpp/src/llama.cpp \
    llama.cpp/src/llama-arch.cpp \
    llama.cpp/src/llama-batch.cpp \
    llama.cpp/src/llama-chat.cpp \
    llama.cpp/src/llama-context.cpp \
    llama.cpp/src/llama-cparams.cpp \
    llama.cpp/src/llama-grammar.cpp \
    llama.cpp/src/llama-hparams.cpp \
    llama.cpp/src/llama-impl.cpp \
    llama.cpp/src/llama-kv-cache.cpp \
    llama.cpp/src/llama-model.cpp \
    llama.cpp/src/llama-model-loader.cpp \
    llama.cpp/src/llama-pos.cpp \
    llama.cpp/src/llama-sampling.cpp \
    llama.cpp/src/llama-vocab.cpp \
    llama.cpp/src/unicode.cpp \
    llama.cpp/src/unicode-data.cpp \
    llama.cpp/src/base64.cpp

# ggml 源文件
LOCAL_SRC_FILES += \
    llama.cpp/ggml/src/ggml.cpp \
    llama.cpp/ggml/src/ggml-alloc.c \
    llama.cpp/ggml/src/ggml-backend.cpp \
    llama.cpp/ggml/src/ggml-backend-reg.cpp \
    llama.cpp/ggml/src/ggml-opt.cpp \
    llama.cpp/ggml/src/ggml-quants.cpp \
    llama.cpp/ggml/src/ggml-threading.cpp \
    llama.cpp/ggml/src/ggml-aarch64.c \
    llama.cpp/ggml/src/ggml-cpu/ggml-cpu.cpp \
    llama.cpp/ggml/src/ggml-cpu/ggml-cpu-aarch64.cpp \
    llama.cpp/ggml/src/ggml-cpu/ggml-cpu-hbm.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/llama.cpp/src \
    $(LOCAL_PATH)/llama.cpp/ggml/include \
    $(LOCAL_PATH)/llama.cpp/ggml/src

LOCAL_CFLAGS := -DGGML_USE_CPU -O2 -std=c11
LOCAL_CPPFLAGS := -std=c++17 -O2
LOCAL_LDLIBS := -llog -landroid

include $(BUILD_SHARED_LIBRARY)

# 编译游戏主库
include $(CLEAR_VARS)
LOCAL_MODULE := adventure-game

LOCAL_SRC_FILES := \
    game_android.c \
    android_wrapper.c \
    ai.c \
    game.c \
    inventory.c \
    npc.c \
    quest.c \
    savegame.c \
    scene.c

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/llama.cpp/src \
    $(LOCAL_PATH)/llama.cpp/ggml/include

LOCAL_CFLAGS := -D__ANDROID__ -Wall -Wextra -O2
LOCAL_CPPFLAGS := -std=c++17 -O2
LOCAL_LDLIBS := -llog -landroid
LOCAL_SHARED_LIBRARIES := llama

include $(BUILD_SHARED_LIBRARY)
