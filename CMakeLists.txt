cmake_minimum_required(VERSION 3.20)

project(vovan-audio DESCRIPTION "C++ vk bot" LANGUAGES CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../build)
set(EXECUTABLE_OUTPUT_PATH ../build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MACOSX_RPATH OFF)
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

file(GLOB_RECURSE SRC "hdr/*.hpp" "src/*.cpp")

add_subdirectory(modules/cpp_vk_lib)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

add_executable(vovan-audio ${SRC})

include_directories(modules/cpp_vk_lib/cpp_vk_lib)
include_directories(modules/cpp_vk_lib/modules/spdlog/include)
include_directories(modules/cpp_vk_lib/modules/asio/asio/include)


target_link_libraries(vovan-audio PRIVATE cpp_vk_lib curlpp)