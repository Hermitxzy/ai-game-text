cmake_minimum_required(VERSION 3.10)
project("adventure-game")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# 编译游戏库（仅游戏核心代码，无 llama.cpp）
add_library(adventure-game SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/game_android.c
)

# 添加头文件路径
target_include_directories(adventure-game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接系统库
target_link_libraries(adventure-game
    PRIVATE
    android
    log
    dl
    m
)
