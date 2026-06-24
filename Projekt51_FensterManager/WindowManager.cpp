#include "WindowManager.h"
#include "MenuPanel.h"
#include "ViewportPanel.h"
#include "EmptyPanel.h"

WindowManager::WindowManager(GLFWwindow* window)
    : m_window(window) {}

WindowManager::~WindowManager() {
    delete m_root;
}

void WindowManager::init() {
    MenuPanel* menu = new MenuPanel(m_window);
    ViewportPanel* viewport = new ViewportPanel(m_window);
    EmptyPanel* props = new EmptyPanel(m_window, "Properties");

    SplitterNode* contentSplit = new SplitterNode(m_window, true, 0.7f);
    contentSplit->setChild1(viewport);
    contentSplit->setChild2(props);

    SplitterNode* rootSplit = new SplitterNode(m_window, false, 0.06f);
    rootSplit->setChild1(menu);
    rootSplit->setChild2(contentSplit);

    m_root = rootSplit;

    int fbW, fbH;
    glfwGetFramebufferSize(m_window, &fbW, &fbH);
    m_root->setRect(0, 0, fbW, fbH);
}

void WindowManager::render() {
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    if (m_root) m_root->render();
}

void WindowManager::update(float dt) {
    if (m_root) m_root->update(dt);
}

void WindowManager::onResize(int w, int h) {
    if (m_root) m_root->setRect(0, 0, w, h);
}

void WindowManager::onMouseButton(int button, int action, int mods) {
    double mx, my;
    glfwGetCursorPos(m_window, &mx, &my);
    int x = (int)mx, y = (int)my;

    if (m_root) {
        SplitterNode* splitter = m_root->findGrabberAt(x, y);
        if (splitter) {
            m_dragSplitter = splitter;
            splitter->onMouseButton(button, action, mods);
            if (action == GLFW_RELEASE) m_dragSplitter = nullptr;
            return;
        }
    }

    Panel* panel = findPanelAt(x, y);
    if (panel) panel->onMouseButton(button, action, mods);
}

void WindowManager::onCursorPos(double x, double y) {
    if (m_dragSplitter) {
        m_dragSplitter->onCursorPos(x, y);
        return;
    }

    if (m_root) {
        Panel* captured = m_root->findCapturedPanel();
        if (captured) {
            captured->onCursorPos(x, y);
            return;
        }
    }

    Panel* panel = findPanelAt((int)x, (int)y);
    if (panel) panel->onCursorPos(x, y);
}

void WindowManager::onScroll(double dx, double dy) {
    double mx, my;
    glfwGetCursorPos(m_window, &mx, &my);
    Panel* panel = findPanelAt((int)mx, (int)my);
    if (panel) panel->onScroll(dx, dy);
}

void WindowManager::onKey(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F4 && (mods & GLFW_MOD_ALT) && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        return;
    }

    if (m_root) {
        Panel* focused = m_root->findKeyboardFocusedPanel();
        if (focused) {
            focused->onKey(key, scancode, action, mods);
            return;
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

Panel* WindowManager::findPanelAt(int x, int y) {
    return m_root ? m_root->findPanelAt(x, y) : nullptr;
}
