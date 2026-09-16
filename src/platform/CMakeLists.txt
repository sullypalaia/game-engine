cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

add_library(engine_window_manager)

target_sources(
  engine_window_manager
  PUBLIC FILE_SET CXX_MODULES FILES engine.platform.window_manager.cppm
  PRIVATE ${PROJECT_SOURCE_DIR}/src/glad.c)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG 3.5.1)

FetchContent_MakeAvailable(glfw)

target_link_libraries(engine_window_manager PUBLIC glfw)
target_include_directories(engine_window_manager
                           PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_features(engine_window_manager PRIVATE cxx_std_20)
