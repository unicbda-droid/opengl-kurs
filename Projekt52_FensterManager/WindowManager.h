#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "SplitterNode.h"
#include "Panel.h"

class WindowManager {
public:
    WindowManager(GLFWwindow* window);
    ~WindowManager();

    void init();
    void update(float dt);
    void render();

    void onResize(int w, int h);
    void onMouseButton(int button, int action, int mods);
    void onCursorPos(double x, double y);
    void onScroll(double dx, double dy);
    void onKey(int key, int scancode, int action, int mods);
    void onChar(unsigned int codepoint);

    Panel* findPanelAt(int x, int y);

private:
    GLFWwindow* m_window;
    SplitterNode* m_root = nullptr;
    SplitterNode* m_dragSplitter = nullptr;
};
