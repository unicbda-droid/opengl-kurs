cmake_minimum_required(VERSION 3.20)
project(Lektion01 VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(Lektion01 main.cpp)

target_link_libraries(Lektion01 PRIVATE OpenGL::GL glfw)
