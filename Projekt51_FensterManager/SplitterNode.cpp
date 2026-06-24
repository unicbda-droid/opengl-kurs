#include "SplitterNode.h"

SplitterNode::SplitterNode(GLFWwindow* window, bool horizontal, float splitRatio)
    : Panel("Splitter", window), m_horizontal(horizontal), m_ratio(splitRatio) {}

SplitterNode::~SplitterNode() {
    delete m_child1;
    delete m_child2;
}

void SplitterNode::setChild1(Panel* child) { m_child1 = child; }
void SplitterNode::setChild2(Panel* child) { m_child2 = child; }

void SplitterNode::setRect(int x, int y, int w, int h) {
    Panel::setRect(x, y, w, h);
    if (w <= 0 || h <= 0) return;

    int halfGrab = GRABBER_SIZE / 2;
    if (m_horizontal) {
        int splitPos = x + (int)(w * m_ratio);
        int leftW = splitPos - x - halfGrab;
        int rightW = x + w - splitPos - halfGrab;
        if (leftW > 0 && m_child1) m_child1->setRect(x, y, leftW, h);
        if (rightW > 0 && m_child2) m_child2->setRect(splitPos + halfGrab, y, rightW, h);
    } else {
        int splitPos = y + (int)(h * m_ratio);
        int topH = splitPos - y - halfGrab;
        int bottomH = y + h - splitPos - halfGrab;
        if (topH > 0 && m_child1) m_child1->setRect(x, y, w, topH);
        if (bottomH > 0 && m_child2) m_child2->setRect(x, splitPos + halfGrab, w, bottomH);
    }
}

void SplitterNode::render() {
    if (m_child1) m_child1->render();
    if (m_child2) m_child2->render();

    Rect g = getGrabberRect();
    if (g.w <= 0 || g.h <= 0) return;

    int fbH;
    glfwGetFramebufferSize(m_window, nullptr, &fbH);

    glDisable(GL_SCISSOR_TEST);
    glViewport(g.x, fbH - g.y - g.h, g.w, g.h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g.w, 0, g.h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.25f, 0.25f, 0.27f);
    glBegin(GL_QUADS);
    glVertex2i(0, 0); glVertex2i(g.w, 0);
    glVertex2i(g.w, g.h); glVertex2i(0, g.h);
    glEnd();

    glColor3f(0.35f, 0.35f, 0.37f);
    glBegin(GL_LINES);
    if (m_horizontal) {
        int cx = g.w / 2;
        glVertex2i(cx, 4); glVertex2i(cx, g.h - 4);
    } else {
        int cy = g.h / 2;
        glVertex2i(4, cy); glVertex2i(g.w - 4, cy);
    }
    glEnd();

    glEnable(GL_SCISSOR_TEST);
}

Panel* SplitterNode::findPanelAt(int x, int y) {
    if (!contains(x, y)) return nullptr;
    Panel* result = nullptr;
    if (m_child1 && m_child1->contains(x, y))
        result = m_child1->findPanelAt(x, y);
    if (!result && m_child2 && m_child2->contains(x, y))
        result = m_child2->findPanelAt(x, y);
    return result;
}

SplitterNode* SplitterNode::findGrabberAt(int x, int y) {
    if (!contains(x, y)) return nullptr;
    Rect g = getGrabberRect();
    if (g.contains(x, y)) return this;
    SplitterNode* result = nullptr;
    if (m_child1) result = m_child1->findGrabberAt(x, y);
    if (!result && m_child2) result = m_child2->findGrabberAt(x, y);
    return result;
}

Panel* SplitterNode::findCapturedPanel() {
    if (isCaptured()) return this;
    Panel* result = nullptr;
    if (m_child1) result = m_child1->findCapturedPanel();
    if (!result && m_child2) result = m_child2->findCapturedPanel();
    return result;
}

Panel* SplitterNode::findKeyboardFocusedPanel() {
    if (hasKeyboardFocus()) return this;
    Panel* result = nullptr;
    if (m_child1) result = m_child1->findKeyboardFocusedPanel();
    if (!result && m_child2) result = m_child2->findKeyboardFocusedPanel();
    return result;
}

void SplitterNode::update(float dt) {
    if (m_child1) m_child1->update(dt);
    if (m_child2) m_child2->update(dt);
}

Rect SplitterNode::getGrabberRect() const {
    if (m_horizontal) {
        int splitPos = m_rect.x + (int)(m_rect.w * m_ratio);
        return {splitPos - GRABBER_SIZE / 2, m_rect.y, GRABBER_SIZE, m_rect.h};
    } else {
        int splitPos = m_rect.y + (int)(m_rect.h * m_ratio);
        return {m_rect.x, splitPos - GRABBER_SIZE / 2, m_rect.w, GRABBER_SIZE};
    }
}

bool SplitterNode::onMouseButton(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            m_dragging = true;
            return true;
        } else if (action == GLFW_RELEASE) {
            m_dragging = false;
            return true;
        }
    }
    return false;
}

bool SplitterNode::onCursorPos(double x, double y) {
    if (!m_dragging) return false;

    int halfGrab = GRABBER_SIZE / 2;
    if (m_horizontal) {
        int totalW = m_rect.w - GRABBER_SIZE;
        if (totalW > 0) {
            m_ratio = (float)((int)x - m_rect.x - halfGrab) / (float)totalW;
            if (m_ratio < 0.1f) m_ratio = 0.1f;
            if (m_ratio > 0.9f) m_ratio = 0.9f;
        }
    } else {
        int totalH = m_rect.h - GRABBER_SIZE;
        if (totalH > 0) {
            m_ratio = (float)((int)y - m_rect.y - halfGrab) / (float)totalH;
            if (m_ratio < 0.1f) m_ratio = 0.1f;
            if (m_ratio > 0.9f) m_ratio = 0.9f;
        }
    }
    setRect(m_rect.x, m_rect.y, m_rect.w, m_rect.h);
    return true;
}
