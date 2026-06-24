#include "ViewportPanel.h"

ViewportPanel::ViewportPanel(GLFWwindow* window)
    : Panel("Viewport", window) {
    m_objects.emplace_back(0.0f, 0.0f, 0.0f);
}

void ViewportPanel::render() {
    if (m_rect.w <= 0 || m_rect.h <= 0) return;

    glEnable(GL_SCISSOR_TEST);
    setupViewport();
    glClearColor(0.12f, 0.12f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = (float)m_rect.w / (float)m_rect.h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (m_camera.isOrtho()) {
        float s = 0.05f;
        if (aspect > 1.0f) glOrtho(-s * aspect, s * aspect, -s, s, 0.1f, 500.0f);
        else glOrtho(-s, s, -s / aspect, s / aspect, 0.1f, 500.0f);
    } else {
        glFrustum(-0.0414f * aspect, 0.0414f * aspect, -0.031f, 0.031f, 0.1f, 500.0f);
    }

    if (m_pick.pending) {
        doPick();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (m_camera.isOrtho()) {
            float s = 0.05f;
            if (aspect > 1.0f) glOrtho(-s * aspect, s * aspect, -s, s, 0.1f, 500.0f);
            else glOrtho(-s, s, -s / aspect, s / aspect, 0.1f, 500.0f);
        } else {
            glFrustum(-0.0414f * aspect, 0.0414f * aspect, -0.031f, 0.031f, 0.1f, 500.0f);
        }
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    m_camera.apply();

    glEnable(GL_DEPTH_TEST);
    drawScene();
    glDisable(GL_DEPTH_TEST);

    glDisable(GL_SCISSOR_TEST);
}

void ViewportPanel::update(float dt) {
    if (!m_hasFocus) return;
    float speed = m_camera.getSpeedMultiplier() * dt * 60.0f;
    auto* w = m_window;

    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) m_camera.move(-speed, 0, 0);
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) m_camera.move(speed, 0, 0);
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) m_camera.move(0, -speed, 0);
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) m_camera.move(0, speed, 0);
    if (glfwGetKey(w, GLFW_KEY_Q) == GLFW_PRESS) m_camera.move(0, 0, speed);
    if (glfwGetKey(w, GLFW_KEY_E) == GLFW_PRESS) m_camera.move(0, 0, -speed);

    // NumPad
    if (glfwGetKey(w, GLFW_KEY_KP_8) == GLFW_PRESS ||
        glfwGetKey(w, GLFW_KEY_KP_4) == GLFW_PRESS ||
        glfwGetKey(w, GLFW_KEY_KP_6) == GLFW_PRESS ||
        glfwGetKey(w, GLFW_KEY_KP_2) == GLFW_PRESS) {
        float orbitSpeed = 60.0f * dt;
        float tx = 0, ty = 0, tz = 0;
        if (m_selectedObject) m_selectedObject->getPosition(tx, ty, tz);
        if (glfwGetKey(w, GLFW_KEY_KP_8) == GLFW_PRESS) m_camera.orbit(0, orbitSpeed, tx, ty, tz);
        if (glfwGetKey(w, GLFW_KEY_KP_2) == GLFW_PRESS) m_camera.orbit(0, -orbitSpeed, tx, ty, tz);
        if (glfwGetKey(w, GLFW_KEY_KP_4) == GLFW_PRESS) m_camera.orbit(-orbitSpeed, 0, tx, ty, tz);
        if (glfwGetKey(w, GLFW_KEY_KP_6) == GLFW_PRESS) m_camera.orbit(orbitSpeed, 0, tx, ty, tz);
    }

    // Gizmo highlight
    if (m_selectedObject && !m_gizmo.isDragging() && !m_mouseCaptured) {
        double mx, my;
        glfwGetCursorPos(w, &mx, &my);
        int fbW, fbH;
        glfwGetFramebufferSize(w, &fbW, &fbH);
        int vp[4] = {m_rect.x, fbH - m_rect.y - m_rect.h, m_rect.w, m_rect.h};
        float ox, oy, oz;
        m_selectedObject->getPosition(ox, oy, oz);
        int axis = m_gizmo.hitTest((float)mx, (float)(m_rect.y + m_rect.h - (int)my),
                                   vp, ox, oy, oz, m_gizmoMode);
        m_gizmo.setAxis(axis);
    } else {
        m_gizmo.setAxis(-1);
    }
}

void ViewportPanel::drawGrid() {
    glColor3f(0.3f, 0.3f, 0.35f);
    glBegin(GL_LINES);
    int half = 25;
    for (int i = -half; i <= half; i++) {
        glVertex3f((float)i, 0.0f, (float)-half);
        glVertex3f((float)i, 0.0f, (float)half);
        glVertex3f((float)-half, 0.0f, (float)i);
        glVertex3f((float)half, 0.0f, (float)i);
    }
    glEnd();
}

void ViewportPanel::drawAxes() {
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0,0,0); glVertex3f(2,0,0);
    glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0,0,0); glVertex3f(0,2,0);
    glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(0,0,0); glVertex3f(0,0,2);
    glEnd();
    glLineWidth(1.0f);
}

void ViewportPanel::drawScene() {
    drawGrid();
    drawAxes();
    for (auto& obj : m_objects) {
        obj.draw(m_viewMode);
    }
    if (m_selectedObject) {
        float ox, oy, oz;
        m_selectedObject->getPosition(ox, oy, oz);
        m_gizmo.render(ox, oy, oz, m_gizmoMode);
    }
}

void ViewportPanel::doPick() {
    int fbW, fbH;
    glfwGetFramebufferSize(m_window, &fbW, &fbH);
    int vy = m_rect.y + m_rect.h - m_pick.y;

    for (size_t i = 0; i < m_objects.size(); i++) {
        unsigned char r = (unsigned char)((i + 1) & 0xFF);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        m_camera.apply();
        glColor3ub(r, 0, 0);
        m_objects[i].draw(ViewMode::Solid);
        glPopMatrix();
    }

    glFlush();
    unsigned char pixel[3] = {0};
    glReadPixels(m_pick.x - m_rect.x, vy, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

    int idx = (int)pixel[0] - 1;
    if (idx >= 0 && idx < (int)m_objects.size())
        selectObject(&m_objects[idx]);
    else
        selectObject(nullptr);

    m_pick.pending = false;
}

void ViewportPanel::selectObject(SceneObject* obj) {
    if (m_selectedObject) m_selectedObject->setSelected(false);
    m_selectedObject = obj;
    if (m_selectedObject) m_selectedObject->setSelected(true);
}

bool ViewportPanel::onMouseButton(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        m_leftDown = (action == GLFW_PRESS);
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
        m_rightDown = (action == GLFW_PRESS);

    if (action == GLFW_PRESS) {
        m_hasFocus = true;
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            m_mouseCaptured = true;
            glfwGetCursorPos(m_window, &m_lastMX, &m_lastMY);
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            return true;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            double mx, my;
            glfwGetCursorPos(m_window, &mx, &my);
            if (!contains((int)mx, (int)my)) return false;

            // Gizmo hit test
            if (m_selectedObject) {
                float ox, oy, oz;
                m_selectedObject->getPosition(ox, oy, oz);
                int fbW, fbH;
                glfwGetFramebufferSize(m_window, &fbW, &fbH);
                int vp[4] = {m_rect.x, fbH - m_rect.y - m_rect.h, m_rect.w, m_rect.h};
                int axis = m_gizmo.hitTest((float)mx, (float)(m_rect.y + m_rect.h - (int)my),
                                           vp, ox, oy, oz, m_gizmoMode);
                if (axis >= 0) {
                    m_gizmo.setActiveAxis(axis);
                    m_gizmo.setDragging(true);
                    m_gizmo.setStartDrag((float)mx, (float)my);
                    return true;
                }
            }

            // Color pick
            m_pick = {(int)mx, (int)my, true};
            return true;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && m_gizmo.isDragging()) {
        m_gizmo.setDragging(false);
        m_gizmo.setActiveAxis(-1);
    }

    if (m_mouseCaptured && !m_leftDown && !m_rightDown) {
        m_mouseCaptured = false;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    return false;
}

bool ViewportPanel::onCursorPos(double x, double y) {
    if (m_mouseCaptured) {
        double dx = x - m_lastMX;
        double dy = y - m_lastMY;
        m_lastMX = x;
        m_lastMY = y;
        if (m_leftDown) {
            m_camera.move(0, (float)dx * 0.05f, (float)-dy * 0.05f);
        } else {
            m_camera.rotate((float)-dx * 0.2f, (float)-dy * 0.2f);
        }
        return true;
    }

    if (m_gizmo.isDragging() && m_selectedObject) {
        float ox, oy, oz;
        m_selectedObject->getPosition(ox, oy, oz);
        float dx, dy, dz;
        m_gizmo.getDragDelta((float)x, (float)y, ox, oy, oz, dx, dy, dz);
        m_selectedObject->setPosition(ox + dx, oy + dy, oz + dz);
        return true;
    }

    return false;
}

bool ViewportPanel::onScroll(double dx, double dy) {
    float speed = 0.5f * m_camera.getSpeedMultiplier();
    m_camera.move(0.0f, 0.0f, (float)dy * speed);
    return true;
}

bool ViewportPanel::onKey(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        m_hasFocus = true;

        // Render modes F1-F4
        if (key == GLFW_KEY_F1) m_viewMode = ViewMode::Solid;
        else if (key == GLFW_KEY_F2) m_viewMode = ViewMode::Wireframe;
        else if (key == GLFW_KEY_F3) m_viewMode = ViewMode::Normal;
        else if (key == GLFW_KEY_F4) m_viewMode = ViewMode::Texture;

        // Gizmo mode
        else if (key == GLFW_KEY_SPACE) {
            if (m_gizmoMode == GizmoMode::Move) m_gizmoMode = GizmoMode::Rotate;
            else if (m_gizmoMode == GizmoMode::Rotate) m_gizmoMode = GizmoMode::Scale;
            else m_gizmoMode = GizmoMode::Move;
        }
        else if (key == GLFW_KEY_G) m_gizmoMode = GizmoMode::Move;
        else if (key == GLFW_KEY_R) m_gizmoMode = GizmoMode::Rotate;
        else if (key == GLFW_KEY_S && !(mods & GLFW_MOD_CONTROL)) m_gizmoMode = GizmoMode::Scale;

        // NumPad views
        else if (key == GLFW_KEY_KP_1) {
            m_camera.lookAt(0, 0, 0, 0, 0, 15);
        } else if (key == GLFW_KEY_KP_3) {
            m_camera.lookAt(0, 0, 0, 90, 0, 15);
        } else if (key == GLFW_KEY_KP_7) {
            m_camera.lookAt(0, 0, 0, 0, 89, 15);
        } else if (key == GLFW_KEY_KP_5) {
            m_camera.setOrtho(!m_camera.isOrtho());
        }

        else if (key == GLFW_KEY_ESCAPE) {
            if (m_mouseCaptured) {
                m_mouseCaptured = false;
                glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                m_hasFocus = false;
            }
        }
    }
    return m_hasFocus;
}
