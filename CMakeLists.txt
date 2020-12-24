cmake_minimum_required(VERSION 3.10)

project(subrosa-server VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB server_src
    "src/*.hpp"
    "src/*.cpp"
)

add_executable(subrosa-server ${server_src})