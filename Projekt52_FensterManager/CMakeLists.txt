cmake_minimum_required(VERSION 3.10)
project(Projekt52_FensterManager VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(imgui CONFIG REQUIRED)
add_executable(Projekt52_FensterManager
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
    MeshData.cpp
    MeshDatabase.cpp
    BitmapFont.cpp
    ConsolePanel.cpp
)
target_link_libraries(Projekt52_FensterManager PRIVATE OpenGL::GL glfw GLEW::GLEW imgui::imgui)

add_executable(test_loopcut
    test_loopcut.cpp
    MeshData.cpp
)
target_link_libraries(test_loopcut PRIVATE OpenGL::GL GLEW::GLEW)

add_executable(test_bridge
    test_bridge.cpp
    MeshData.cpp
)
target_link_libraries(test_bridge PRIVATE OpenGL::GL GLEW::GLEW)

add_executable(test_inset
    test_inset.cpp
    MeshData.cpp
)
target_link_libraries(test_inset PRIVATE OpenGL::GL GLEW::GLEW)

add_executable(test_mirrorweld
    test_mirrorweld.cpp
    MeshData.cpp
)
target_link_libraries(test_mirrorweld PRIVATE OpenGL::GL GLEW::GLEW)

add_executable(test_facesel
    test_facesel.cpp
    MeshData.cpp
)
target_link_libraries(test_facesel PRIVATE OpenGL::GL GLEW::GLEW)

add_executable(demo_all_features
    demo_all_features.cpp
    MeshData.cpp
)
target_link_libraries(demo_all_features PRIVATE OpenGL::GL GLEW::GLEW)
