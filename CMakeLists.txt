cmake_minimum_required(VERSION 3.8)

add_subdirectory(libs)

project(chat_server)
project(chat_client)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 

find_package(Boost COMPONENTS system REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(common)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include/common" chat_common_includes)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src/common" chat_common_sources)
find_ui_files("${CMAKE_CURRENT_SOURCE_DIR}/ui/common" chat_common_uis)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include/server" chat_server_includes)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src/server" chat_server_sources)
find_ui_files("${CMAKE_CURRENT_SOURCE_DIR}/ui/server" chat_server_uis)

find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/include/client" chat_client_includes)
find_c_and_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/src/client" chat_client_sources)
find_ui_files("${CMAKE_CURRENT_SOURCE_DIR}/ui/client" chat_client_uis)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
        ${chat_common_includes} ${chat_common_sources} ${chat_common_uis}
        ${chat_server_includes} ${chat_server_sources} ${chat_server_uis}
        ${chat_client_includes} ${chat_client_sources} ${chat_client_uis})

add_executable(chat_server
        ${chat_common_includes} ${chat_common_sources}
        ${chat_server_includes} ${chat_server_sources}
        ${chat_common_uis} ${chat_server_uis})
target_include_directories(chat_server PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/build/
        ${Boost_INCLUDE_DIRS})
target_link_libraries(chat_server PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets ${Boost_LIBRARIES} serialization wsock32 ws2_32)

add_executable(chat_client
        ${chat_common_includes} ${chat_common_sources}
        ${chat_client_includes} ${chat_client_sources}
        ${chat_common_uis} ${chat_client_uis})
target_include_directories(chat_client PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/build/
        ${Boost_INCLUDE_DIRS})
target_link_libraries(chat_client PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets ${Boost_LIBRARIES} serialization)

if(MINGW)
    target_link_libraries(chat_client PUBLIC ws2_32)
    target_link_libraries(chat_server PUBLIC ws2_32)
endif()

set_target_properties(chat_server
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
set_target_properties(chat_server PROPERTIES CXX_STANDARD 20)
set_target_properties(chat_server PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(chat_server PROPERTIES FOLDER chat)

set_target_properties(chat_client
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
set_target_properties(chat_client PROPERTIES CXX_STANDARD 20)
set_target_properties(chat_client PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(chat_client PROPERTIES FOLDER chat)