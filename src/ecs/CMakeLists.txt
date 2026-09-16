cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

add_library(engine_ecs)

target_sources(engine_ecs PUBLIC FILE_SET CXX_MODULES FILES
                                 engine.ecs.components.cppm engine.ecs.cppm)

include(FetchContent)

FetchContent_Declare(
  EnTT
  GIT_REPOSITORY https://github.com/skypjack/entt
  GIT_TAG v3.16.0)

FetchContent_MakeAvailable(EnTT)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG 1.0.3)

FetchContent_MakeAvailable(glm)

target_link_libraries(engine_ecs PRIVATE EnTT::EnTT glm::glm)
target_compile_features(engine_ecs PRIVATE cxx_std_20)
