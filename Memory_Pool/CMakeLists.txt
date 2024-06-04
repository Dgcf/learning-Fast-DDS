cmake_minimum_required(VERSION 3.20)

project(memory_pool)

set(CMAKE_BUILD_TYPE "Debug")
set(TARGET memorypool)

file(GLOB SOURCE_FILE1 ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB SOURCE_FILE2 ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_library(
    ${TARGET}
    SHARED
    ${SOURCE_FILE1}
    ${SOURCE_FILE2}
)

target_include_directories(
    ${TARGET}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
