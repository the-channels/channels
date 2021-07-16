cmake_minimum_required(VERSION 3.16)
project(channel_hub)

include_directories(src)
include_directories(../proto)
include_directories(curl/include)
include_directories(curlpp/include)
include_directories(json/single_include/nlohmann)

add_definitions(-DCHANNELS_PROTO_SERVER)

file(GLOB_RECURSE CHALLE_HUB_SRC "src/*.h" "src/*.cpp" "../proto/*.c")
set(CMAKE_CXX_STANDARD 11)

add_executable(channel_hub ${CHALLE_HUB_SRC})

add_custom_target(copy_configurations ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/configurations"
    "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_custom_target(create_cache_folder ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/bin/cache")

if(WIN32)

add_custom_target(copy_sdl ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/sdl"
    "${CMAKE_CURRENT_SOURCE_DIR}/bin")

add_dependencies(channel_hub copy_sdl)

target_link_libraries(channel_hub legacy_stdio_definitions.lib)
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
else()
set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()

set(OPENSSL_USE_STATIC_LIBS TRUE)
set(CURLPP_BUILD_SHARED_LIBS OFF CACHE BOOL "No shareds thanks")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "No shareds thanks")
set(CPR_BUILD_TESTS OFF CACHE BOOL "No tests thanks")
set(CPR_ENABLE_SSL ON CACHE BOOL "SSL Please")
set(CURL_DISABLE_TESTS TRUE)
find_package(OpenSSL REQUIRED)

add_subdirectory(cpr)

include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(channel_hub ${OPENSSL_LIBRARIES} cpr::cpr)

set_target_properties(channel_hub
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../../SDL2")

add_subdirectory(img2spec)

set_target_properties(img2spec
    PROPERTIES
    CMAKE_EXE_LINKER_FLAGS "-static"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

add_dependencies(channel_hub img2spec copy_configurations create_cache_folder)