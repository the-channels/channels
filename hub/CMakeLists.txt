cmake_minimum_required(VERSION 3.16)
project(channels_hub)

include_directories(src)
include_directories(../proto)
include_directories(curl/include)
include_directories(curlpp/include)
include_directories(json/single_include/nlohmann)

add_definitions(-DCHANNELS_PROTO_SERVER)

file(GLOB_RECURSE CHALLE_HUB_SRC "src/*.h" "src/*.cpp" "../proto/*.c")
set(CMAKE_CXX_STANDARD 11)

add_executable(channels_hub ${CHALLE_HUB_SRC})

add_custom_target(copy_configurations ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/configurations"
    "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_custom_target(create_cache_folder ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/bin/cache")

if(WIN32)
target_link_libraries(channels_hub legacy_stdio_definitions.lib wsock32 ws2_32)
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
else()
set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()

set(OPENSSL_USE_STATIC_LIBS TRUE)
set(CURLPP_BUILD_SHARED_LIBS OFF CACHE BOOL "No shareds thanks")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "No shareds thanks")
set(CURL_DISABLE_TESTS TRUE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set_target_properties(channels_hub
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

include_directories(img2spec/include)
SET(IMG2SPEC_BUILD_EXE OFF CACHE BOOL "Do not build executable of img2spec")
SET(IMG2SPEC_BUILD_STATIC ON CACHE BOOL "Build static library of img2spec")

add_subdirectory(img2spec)
add_subdirectory(pybind11)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

if(SKIP_PACKAGE_DEVELOP)
    message(WARNING "Skipped setup.py develop")
else()
    add_custom_target(
        channel_packages
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/channels/setup.py develop
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/channels
        COMMENT "Installing channels develop)"
    )
    add_dependencies(channels_hub channel_packages)
endif()

add_dependencies(channels_hub img2spec_static copy_configurations create_cache_folder)

target_link_libraries(channels_hub img2spec_static pybind11::embed)
