cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

add_library(engine_assets)

target_sources(
  engine_assets
  PUBLIC FILE_SET
         public_modules
         TYPE
         CXX_MODULES
         BASE_DIRS
         ${PROJECT_SOURCE_DIR}
         FILES
         engine.assets.importer.cppm)

include(FetchContent)

set(ASSIMP_BUILD_ZLIB
    ON
    CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER
    ON
    CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER
    ON
    CACHE BOOL "" FORCE)

set(ASSIMP_NO_EXPORT
    ON
    CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS
    OFF
    CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DOCS
    OFF
    CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL
    OFF
    CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_DRACO
    OFF
    CACHE BOOL "" FORCE)

FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG v6.0.5)

FetchContent_MakeAvailable(assimp)

target_link_libraries(engine_assets PRIVATE assimp::assimp EnTT::EnTT
                                            engine_ecs)
target_compile_features(engine_assets PRIVATE cxx_std_20)
