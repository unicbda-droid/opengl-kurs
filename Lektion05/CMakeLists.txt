cmake_minimum_required(VERSION 3.20)
project(Lektion05 VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(Lektion05 main.cpp)

target_link_libraries(Lektion05 PRIVATE OpenGL::GL glfw)
