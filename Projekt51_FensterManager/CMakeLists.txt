cmake_minimum_required(VERSION 3.10)
project(Projekt51_FensterManager VERSION 1.1)
set(CMAKE_CXX_STANDARD 17)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(imgui CONFIG REQUIRED)
add_executable(Projekt51_FensterManager
    main.cpp
    Panel.cpp
    Camera.cpp
    SplitterNode.cpp
    ViewportPanel.cpp
    MenuPanel.cpp
    EmptyPanel.cpp
    WindowManager.cpp
    SceneObject.cpp
    Gizmo.cpp
)
target_link_libraries(Projekt51_FensterManager PRIVATE OpenGL::GL glfw GLEW::GLEW imgui::imgui)
