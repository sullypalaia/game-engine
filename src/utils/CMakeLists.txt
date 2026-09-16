cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

add_library(engine_utils)

target_sources(engine_utils PRIVATE FILE_SET CXX_MODULES FILES
                                    engine.utils.concepts.cppm)

target_compile_features(engine_utils PRIVATE cxx_std_20)
