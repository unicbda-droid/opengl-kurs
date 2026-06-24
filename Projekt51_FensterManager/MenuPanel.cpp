#include "MenuPanel.h"

MenuPanel::MenuPanel(GLFWwindow* window) : Panel("Menu", window) {
    m_items = {{"Datei", {}}, {"Bearbeiten", {}}, {"Ansicht", {}}, {"Viewport", {}}, {"Hilfe", {}}};
}

void MenuPanel::render() {
    if (m_rect.w <= 0 || m_rect.h <= 0) return;

    glEnable(GL_SCISSOR_TEST);
    setupViewport();
    glClearColor(0.22f, 0.22f, 0.24f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_rect.w, 0, m_rect.h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    int itemX = 4;
    float itemW = 80.0f;
    int itemH = m_rect.h - 4;

    for (size_t i = 0; i < m_items.size(); i++) {
        Rect r = {itemX, 2, (int)itemW - 4, itemH};
        m_items[i].rect = r;

        glColor3f((int)i == m_hovered ? 0.3f : 0.22f,
                  (int)i == m_hovered ? 0.3f : 0.22f,
                  (int)i == m_hovered ? 0.35f : 0.24f);
        glBegin(GL_QUADS);
        glVertex2i(r.x, r.y); glVertex2i(r.x + r.w, r.y);
        glVertex2i(r.x + r.w, r.y + r.h); glVertex2i(r.x, r.y + r.h);
        glEnd();

        glColor3f(0.7f, 0.7f, 0.7f);
        int ty = r.y + 12;
        glBegin(GL_LINES);
        glVertex2i(r.x + 6, ty); glVertex2i(r.x + r.w - 6, ty);
        glVertex2i(r.x + 6, ty + 5); glVertex2i(r.x + r.w - 6, ty + 5);
        glVertex2i(r.x + 6, ty + 10); glVertex2i(r.x + r.w - 6, ty + 10);
        glEnd();

        itemX += (int)itemW;
    }

    glDisable(GL_SCISSOR_TEST);
}

bool MenuPanel::onMouseButton(int button, int action, int mods) {
    return false;
}

bool MenuPanel::onCursorPos(double x, double y) {
    int lx = (int)x - m_rect.x;
    int ly = (int)y - m_rect.y;
    m_hovered = -1;
    for (size_t i = 0; i < m_items.size(); i++) {
        if (m_items[i].rect.contains(lx, ly)) {
            m_hovered = (int)i;
            break;
        }
    }
    return false;
}
