cmake_minimum_required(VERSION 3.8)
project(serialization_tests)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include" serialization_tests_includes)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src" serialization_tests_sources)

add_executable(serialization_tests ${serialization_tests_includes} ${serialization_tests_sources})
target_link_libraries(serialization_tests PUBLIC serialization libgtest libgmock)

set_target_properties(serialization_tests PROPERTIES CXX_STANDARD 20)
set_target_properties(serialization_tests PROPERTIES LINKER_LANGUAGE CXX)