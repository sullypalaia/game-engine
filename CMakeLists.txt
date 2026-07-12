cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(game_engine)

add_subdirectory(src)

add_executable(engine)

target_sources(engine PRIVATE src/example1.cpp src/glad.c)
target_include_directories(engine PUBLIC include)

target_link_libraries(engine PRIVATE core_engine)
