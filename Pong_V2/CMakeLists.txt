cmake_minimum_required(VERSION 3.20)
project(Pong_V2 VERSION 2.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(Pong main.cpp)
target_link_libraries(Pong PRIVATE OpenGL::GL glfw)
