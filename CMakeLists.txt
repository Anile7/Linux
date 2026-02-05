cmake_minimum_required(VERSION 3.10)
project(KubSH CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")

file(GLOB SOURCES "src/*.cpp")

add_executable(kubsh ${SOURCES})

target_link_libraries(kubsh PRIVATE readline fuse3)
