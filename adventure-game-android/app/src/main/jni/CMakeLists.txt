cmake_minimum_required(VERSION 3.10)
project("adventure-game")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# 编译游戏库（包含核心逻辑和存档系统）
add_library(adventure-game SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/game_android.c
)

# 链接系统库
target_link_libraries(adventure-game
    PRIVATE
    android
    log
    dl
    m
)
