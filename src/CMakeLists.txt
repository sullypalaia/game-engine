include(FetchContent)

FetchContent_Declare(
  EnTT
  GIT_REPOSITORY https://github.com/skypjack/entt
  GIT_TAG v3.16.0
)

FetchContent_MakeAvailable(EnTT)

find_package(glfw3 3.3 REQUIRED)

add_library(core_engine)
target_sources(core_engine PUBLIC FILE_SET CXX_MODULES BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES Camera.cppm renderers/OpenGL/ShaderGen.cppm Components.cppm
  renderers/OpenGL/Groups.cppm ECS.cppm Meshes.cppm Shaders.cppm WindowManager.cppm Concepts.cppm renderers/OpenGL/OpenGL.cppm renderers/OpenGL/Buffers.cppm)
target_include_directories(core_engine PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(core_engine PUBLIC EnTT::EnTT glfw)
