cmake_minimum_required(VERSION 3.8)
project(serialization)

enable_testing()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(common)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include" serialization_includes)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src" serialization_sources)

add_library(serialization STATIC ${serialization_includes} ${serialization_sources})
target_include_directories(serialization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(serialization PROPERTIES CXX_STANDARD 20)
set_target_properties(serialization PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(tests)