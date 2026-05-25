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
