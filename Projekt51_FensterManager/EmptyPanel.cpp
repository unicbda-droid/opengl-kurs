#include "EmptyPanel.h"

EmptyPanel::EmptyPanel(GLFWwindow* window, const std::string& label)
    : Panel(label, window), m_label(label) {}

void EmptyPanel::render() {
    if (m_rect.w <= 0 || m_rect.h <= 0) return;

    glEnable(GL_SCISSOR_TEST);
    setupViewport();
    glClearColor(0.17f, 0.17f, 0.19f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_rect.w, 0, m_rect.h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.2f, 0.2f, 0.22f);
    glBegin(GL_QUADS);
    glVertex2i(0, m_rect.h - 24); glVertex2i(m_rect.w, m_rect.h - 24);
    glVertex2i(m_rect.w, m_rect.h); glVertex2i(0, m_rect.h);
    glEnd();

    glColor3f(0.35f, 0.35f, 0.38f);
    int y = m_rect.h - 40;
    for (int i = 0; i < 6; i++) {
        glBegin(GL_LINES);
        glVertex2i(8, y); glVertex2i(m_rect.w - 8, y);
        glEnd();
        y -= 25;
    }

    glDisable(GL_SCISSOR_TEST);
}
