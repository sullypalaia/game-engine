# this is currently used to build the examples but will eventually be used to
# build the engine as a library so it can be linked in projects

cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(game_engine)

add_subdirectory(src/ecs)
add_subdirectory(src/scene)
add_subdirectory(src/platform)
add_subdirectory(src/graphics/apis/opengl)
add_subdirectory(src/utils)

# entt is added here for use with the example but shouldn't be in real projects
FetchContent_Declare(
  EnTT
  GIT_REPOSITORY https://github.com/skypjack/entt
  GIT_TAG v3.16.0)

FetchContent_MakeAvailable(EnTT)

add_executable(engine)

target_sources(engine PRIVATE src/examples/example3.cpp)
target_compile_features(engine PRIVATE cxx_std_20)
target_include_directories(engine PRIVATE include)

target_link_libraries(
  engine PRIVATE engine_ecs engine_scene engine_window_manager
                 engine_opengl_backend engine_utils EnTT::EnTT)
