#include "Panel.h"

Panel::Panel(const std::string& name, GLFWwindow* window)
    : m_name(name), m_window(window) {}

void Panel::setRect(int x, int y, int w, int h) {
    m_rect = {x, y, w, h};
}

Panel* Panel::findPanelAt(int x, int y) {
    return contains(x, y) ? this : nullptr;
}

SplitterNode* Panel::findGrabberAt(int x, int y) {
    return nullptr;
}

Panel* Panel::findCapturedPanel() {
    return isCaptured() ? this : nullptr;
}

Panel* Panel::findKeyboardFocusedPanel() {
    return hasKeyboardFocus() ? this : nullptr;
}

void Panel::setupViewport() {
    int fbW, fbH;
    glfwGetFramebufferSize(m_window, &fbW, &fbH);
    glViewport(m_rect.x, fbH - m_rect.y - m_rect.h, m_rect.w, m_rect.h);
    glScissor(m_rect.x, fbH - m_rect.y - m_rect.h, m_rect.w, m_rect.h);
}
