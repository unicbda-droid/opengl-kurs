cmake_minimum_required(VERSION 3.20)
project(Vulkan01 VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(Vulkan01 main.cpp)

target_link_libraries(Vulkan01 PRIVATE Vulkan::Vulkan glfw)
