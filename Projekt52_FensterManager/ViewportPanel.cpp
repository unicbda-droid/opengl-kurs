#include "ViewportPanel.h"
#include <windows.h>
#include <commdlg.h>
#include <filesystem>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <ctime>
#include <unordered_map>

namespace fs = std::filesystem;

static std::string openFileDialog() {
    OPENFILENAMEA ofn = {0};
    char path[MAX_PATH] = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFilter = "OBJ Files\0*.obj\0All Files\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) return std::string(path);
    return "";
}

ViewportPanel::ViewportPanel(GLFWwindow* window)
    : Panel("Viewport", window) {
    m_objects.emplace_back(0.0f, 0.0f, 0.0f, "Default_Cube");
    loadOBJDirectory("objs");
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

    // Lighting setup
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    GLfloat lightPos[] = { 15.0f, 25.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    GLfloat lightAmb[] = { 0.25f, 0.25f, 0.3f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    GLfloat lightDiff[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);

    glEnable(GL_DEPTH_TEST);
    drawScene();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    if (m_screenshotPending) {
        saveScreenshot();
        m_screenshotPending = false;
    }

    glDisable(GL_SCISSOR_TEST);
}

void ViewportPanel::update(float dt) {
    if (!m_keyboardFocus) return;
    float speed = m_camera.getSpeedMultiplier() * dt * 60.0f;
    auto* w = m_window;

    if (m_mouseCaptured) {
        if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) m_camera.move(-speed, 0, 0);
        if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) m_camera.move(speed, 0, 0);
        if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) m_camera.move(0, -speed, 0);
        if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) m_camera.move(0, speed, 0);
        if (glfwGetKey(w, GLFW_KEY_Q) == GLFW_PRESS) m_camera.move(0, 0, speed);
        if (glfwGetKey(w, GLFW_KEY_E) == GLFW_PRESS) m_camera.move(0, 0, -speed);
    }

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

    m_shiftDown = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                  glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    // Gizmo highlight
    if (m_selectedObject && !m_gizmo.isDragging() && !m_mouseCaptured) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        float aspect = (float)m_rect.w / (float)m_rect.h;
        if (m_camera.isOrtho()) {
            float s = 0.05f;
            if (aspect > 1.0f) glOrtho(-s * aspect, s * aspect, -s, s, 0.1f, 500.0f);
            else glOrtho(-s, s, -s / aspect, s / aspect, 0.1f, 500.0f);
        } else {
            glFrustum(-0.0414f * aspect, 0.0414f * aspect, -0.031f, 0.031f, 0.1f, 500.0f);
        }
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        m_camera.apply();

        double mx, my;
        glfwGetCursorPos(w, &mx, &my);
        int fbW, fbH;
        glfwGetFramebufferSize(w, &fbW, &fbH);
        int vp[4] = {m_rect.x, fbH - m_rect.y - m_rect.h, m_rect.w, m_rect.h};

        float ox, oy, oz;
        if (m_editMode) {
            m_selectedObject->mesh().getSelectionCenter(ox, oy, oz);
            float px, py, pz;
            m_selectedObject->getPosition(px, py, pz);
            ox += px; oy += py; oz += pz;
        } else {
            m_selectedObject->getPosition(ox, oy, oz);
        }
        int axis = m_gizmo.hitTest((float)mx, (float)(m_rect.y + m_rect.h - (int)my),
                                   vp, ox, oy, oz, m_gizmoMode);
        m_gizmo.setAxis(axis);

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
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
    for (size_t i = 0; i < m_objects.size(); i++) {
        float r = 0.7f + (i % 3) * 0.1f;
        float g = 0.7f + ((i + 1) % 3) * 0.1f;
        float b = 0.8f + ((i + 2) % 3) * 0.1f;
        glColor3f(r, g, b);
        m_objects[i].draw(m_viewMode);
    }
    if (m_selectedObject) {
        float ox, oy, oz;
        if (m_editMode) {
            m_selectedObject->mesh().getSelectionCenter(ox, oy, oz);
            float px, py, pz;
            m_selectedObject->getPosition(px, py, pz);
            ox += px; oy += py; oz += pz;
        } else {
            m_selectedObject->getPosition(ox, oy, oz);
        }
        m_gizmo.render(ox, oy, oz, m_gizmoMode);
    }
}

// Edge pick helpers
static float pointSegDist2D(float px, float py, float ax, float ay, float bx, float by) {
    float dx = bx - ax, dy = by - ay;
    float len2 = dx * dx + dy * dy;
    if (len2 < 1e-6f) return sqrtf((px - ax) * (px - ax) + (py - ay) * (py - ay));
    float t = fmaxf(0.0f, fminf(1.0f, ((px - ax) * dx + (py - ay) * dy) / len2));
    float cx = ax + t * dx, cy = ay + t * dy;
    return sqrtf((px - cx) * (px - cx) + (py - cy) * (py - cy));
}

static void projScreen(float x, float y, float z,
                       const GLdouble* model, const GLdouble* proj, const GLint* view,
                       float& sx, float& sy) {
    double vx = model[0]*x + model[4]*y + model[8]*z + model[12];
    double vy = model[1]*x + model[5]*y + model[9]*z + model[13];
    double vz = model[2]*x + model[6]*y + model[10]*z + model[14];
    double vw = model[3]*x + model[7]*y + model[11]*z + model[15];
    double cx = proj[0]*vx + proj[4]*vy + proj[8]*vz + proj[12]*vw;
    double cy = proj[1]*vx + proj[5]*vy + proj[9]*vz + proj[13]*vw;
    double cz = proj[2]*vx + proj[6]*vy + proj[10]*vz + proj[14]*vw;
    double cw = proj[3]*vx + proj[7]*vy + proj[11]*vz + proj[15]*vw;
    if (cw == 0) { sx = sy = -1; return; }
    sx = (float)(view[0] + (cx / cw + 1) * view[2] / 2);
    sy = (float)(view[1] + (cy / cw + 1) * view[3] / 2);
}

void ViewportPanel::doPick() {
    int fbW, fbH;
    glfwGetFramebufferSize(m_window, &fbW, &fbH);
    int vy = m_rect.y + m_rect.h - m_pick.y;

    if (m_editMode && m_selectedObject) {
        float px, py, pz;
        m_selectedObject->getPosition(px, py, pz);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        m_camera.apply();
        float yaw, pitch, roll;
        m_selectedObject->getRotation(yaw, pitch, roll);
        glTranslatef(px, py, pz);
        glRotatef(pitch, 1, 0, 0);
        glRotatef(yaw, 0, 1, 0);
        glRotatef(roll, 0, 0, 1);
        glScalef(m_selectedObject->getScale(), m_selectedObject->getScale(), m_selectedObject->getScale());

        // Read modelview for edge projection
        GLdouble model[16], proj[16];
        GLint view[4];
        glGetDoublev(GL_MODELVIEW_MATRIX, model);
        glGetDoublev(GL_PROJECTION_MATRIX, proj);
        glGetIntegerv(GL_VIEWPORT, view);

        // Vertex color pick
        glPointSize(10.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < m_selectedObject->mesh().vertexCount(); i++) {
            unsigned char r = (unsigned char)((i + 1) & 0xFF);
            glColor3ub(r, 0, 0);
            float vx, vy, vz;
            m_selectedObject->mesh().getVertex(i, vx, vy, vz);
            glVertex3f(vx, vy, vz);
        }
        glEnd();
        glPointSize(1.0f);
        glPopMatrix();

        glFlush();
        unsigned char pixel[3] = {0};
        glReadPixels(m_pick.x - m_rect.x, vy, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

        int idx = (int)pixel[0] - 1;
        auto& mesh = m_selectedObject->mesh();

        if (!m_shiftDown) mesh.clearSelection();
        if (idx >= 0 && idx < mesh.vertexCount()) {
            mesh.clearFaceSelection();
            mesh.select(idx, !mesh.isSelected(idx));
        } else {
            // No vertex hit → try edge picking via screen-space distance
            float cx = (float)(m_pick.x - m_rect.x);
            float cy = (float)(m_pick.y - 0); // already in framebuffer coords
            float bestDist = 20.0f;
            int e0 = -1, e1 = -1;
            auto& verts = mesh.vertices();
            auto& inds = mesh.indices();
            // deduplicate edges
            struct EK { int a, b; };
            auto mkK = [](int a, int b) { if (a < b) return EK{a,b}; return EK{b,a}; };
            auto hsh = [](const EK& k) { return (unsigned)k.a * 73856093u ^ (unsigned)k.b * 19349663u; };
            auto eql = [](const EK& a, const EK& b) { return a.a == b.a && a.b == b.b; };
            std::unordered_map<EK, int, decltype(hsh), decltype(eql)> done(0, hsh, eql);
            for (size_t fi = 0; fi < inds.size(); fi += 4) {
                for (int c = 0; c < 4; c++) {
                    int va = inds[fi + c], vb = inds[fi + (c + 1) % 4];
                    EK k = mkK(va, vb);
                    if (done.count(k)) continue;
                    done[k] = 1;
                    float sax, say, sbx, sby;
                    projScreen(verts[va].px, verts[va].py, verts[va].pz, model, proj, view, sax, say);
                    projScreen(verts[vb].px, verts[vb].py, verts[vb].pz, model, proj, view, sbx, sby);
                    float d = pointSegDist2D(cx, cy, sax, say, sbx, sby);
                    if (d < bestDist) { bestDist = d; e0 = va; e1 = vb; }
                }
            }
            if (e0 >= 0 && e1 >= 0) {
                mesh.clearSelection();
                mesh.clearFaceSelection();
                mesh.select(e0, true);
                mesh.select(e1, true);
            } else {
                // try face picking
                int fi = mesh.pickFace(m_pick.x - m_rect.x, m_pick.y, m_rect.w, m_rect.h);
                if (fi >= 0) {
                    mesh.clearSelection();
                    mesh.selectFace(fi);
                }
            }
        }
    } else {
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
    }

    m_pick.pending = false;
}

void ViewportPanel::selectObject(SceneObject* obj) {
    if (m_selectedObject) m_selectedObject->setSelected(false);
    m_selectedObject = obj;
    if (m_selectedObject) m_selectedObject->setSelected(true);
}

void ViewportPanel::saveScreenshot() {
    int fbW, fbH;
    glfwGetFramebufferSize(m_window, &fbW, &fbH);
    int w = m_rect.w, h = m_rect.h;
    if (w <= 0 || h <= 0) return;

    std::vector<unsigned char> pixels(w * h * 3);
    glReadPixels(m_rect.x, fbH - m_rect.y - h, w, h, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());

    // BMP file: 14 header + 40 DIB + pixels (bottom-up = flip y)
    int rowSize = ((w * 24 + 31) / 32) * 4; // BMP row padding to 4 bytes
    int dataSize = rowSize * h;
    int fileSize = 14 + 40 + dataSize;

    std::vector<unsigned char> bmp(fileSize);
    // File header
    bmp[0] = 'B'; bmp[1] = 'M';
    *(int*)&bmp[2] = fileSize;
    *(int*)&bmp[10] = 14 + 40;
    // DIB header
    *(int*)&bmp[14] = 40;
    *(int*)&bmp[18] = w;
    *(int*)&bmp[22] = h;
    *(short*)&bmp[26] = 1;
    *(short*)&bmp[28] = 24;
    *(int*)&bmp[30] = 0; // no compression

    // Pixel data (bottom-up)
    for (int y = 0; y < h; y++) {
        int srcLine = (h - 1 - y) * w * 3; // flip Y
        int dstOff = 14 + 40 + y * rowSize;
        memcpy(&bmp[dstOff], &pixels[srcLine], w * 3);
    }

    char path[256];
    snprintf(path, sizeof(path), "screenshot_%04d.bmp", (int)time(nullptr) % 10000);
    FILE* f = fopen(path, "wb");
    if (f) {
        fwrite(bmp.data(), 1, fileSize, f);
        fclose(f);
        printf("Screenshot: %s (%dx%d)\n", path, w, h);
    }
}

int ViewportPanel::getEditFaceUnderCursor() {
    if (!m_selectedObject) return -1;
    // Face color picking
    int fbW, fbH;
    glfwGetFramebufferSize(m_window, &fbW, &fbH);
    double mx, my;
    glfwGetCursorPos(m_window, &mx, &my);
    int vx = (int)mx - m_rect.x;
    int vy = m_rect.y + m_rect.h - (int)my;
    if (vx < 0 || vx >= m_rect.w || vy < 0 || vy >= m_rect.h) return -1;

    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setupViewport();
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
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    m_camera.apply();

    float px, py, pz;
    m_selectedObject->getPosition(px, py, pz);
    float yaw, pitch, roll;
    m_selectedObject->getRotation(yaw, pitch, roll);
    glTranslatef(px, py, pz);
    glRotatef(pitch, 1, 0, 0);
    glRotatef(yaw, 0, 1, 0);
    glRotatef(roll, 0, 0, 1);
    glScalef(m_selectedObject->getScale(), m_selectedObject->getScale(), m_selectedObject->getScale());

    auto& mesh = m_selectedObject->mesh();
    glBegin(GL_QUADS);
    for (int i = 0; i < mesh.faceCount(); i++) {
        unsigned char r = (unsigned char)((i + 1) & 0xFF);
        glColor3ub(r, 0, 0);
        for (int j = 0; j < 4; j++) {
            float vx, vy, vz;
            mesh.getVertex(mesh.faceVertex(i, j), vx, vy, vz);
            glVertex3f(vx, vy, vz);
        }
    }
    glEnd();
    glFlush();
    glEnable(GL_SCISSOR_TEST);

    unsigned char pixel[3] = {0};
    glReadPixels(vx, vy, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    int idx = (int)pixel[0] - 1;
    if (idx >= 0 && idx < mesh.faceCount()) return idx;
    return -1;
}

bool ViewportPanel::onMouseButton(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        m_leftDown = (action == GLFW_PRESS);
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
        m_rightDown = (action == GLFW_PRESS);

    if (action == GLFW_PRESS) {
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
                if (m_editMode) {
                    m_selectedObject->mesh().getSelectionCenter(ox, oy, oz);
                    float px, py, pz;
                    m_selectedObject->getPosition(px, py, pz);
                    ox += px; oy += py; oz += pz;
                } else {
                    m_selectedObject->getPosition(ox, oy, oz);
                }
                int fbW, fbH;
                glfwGetFramebufferSize(m_window, &fbW, &fbH);
                int vp[4] = {m_rect.x, fbH - m_rect.y - m_rect.h, m_rect.w, m_rect.h};
                int axis = m_gizmo.hitTest((float)mx, (float)(m_rect.y + m_rect.h - (int)my),
                                           vp, ox, oy, oz, m_gizmoMode);
                if (axis >= 0) {
                    m_gizmo.setActiveAxis(axis);
                    m_gizmo.setDragging(true);
                    m_gizmo.setStartDrag((float)mx, (float)my);
                    m_gizmo.setPos((float)mx, (float)my);
                    return true;
                }
            }

            // Object/vertex pick
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
        float dx, dy, dz;
        if (m_editMode) {
            float ox, oy, oz;
            m_selectedObject->mesh().getSelectionCenter(ox, oy, oz);
            float px, py, pz;
            m_selectedObject->getPosition(px, py, pz);
            m_gizmo.getDragDelta((float)x, (float)y, ox+px, oy+py, oz+pz, dx, dy, dz);
            m_selectedObject->mesh().translateSelected(dx, dy, dz);
        } else {
            float ox, oy, oz;
            m_selectedObject->getPosition(ox, oy, oz);
            m_gizmo.getDragDelta((float)x, (float)y, ox, oy, oz, dx, dy, dz);
            m_selectedObject->setPosition(ox + dx, oy + dy, oz + dz);
        }
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

        // Render modes F1-F4
        if (key == GLFW_KEY_F1) m_viewMode = ViewMode::Solid;
        else if (key == GLFW_KEY_F2) m_viewMode = ViewMode::Wireframe;
        else if (key == GLFW_KEY_F3) m_viewMode = ViewMode::Normal;
        else if (key == GLFW_KEY_F4) m_viewMode = ViewMode::Texture;

        // Edit mode
        else if (key == GLFW_KEY_TAB) {
            m_editMode = !m_editMode;
            if (m_selectedObject) m_selectedObject->mesh().clearSelection();
        }

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
        else if (key == GLFW_KEY_KP_1) { m_camera.lookAt(0, 0, 0, 0, 0, 15); }
        else if (key == GLFW_KEY_KP_3) { m_camera.lookAt(0, 0, 0, 90, 0, 15); }
        else if (key == GLFW_KEY_KP_7) { m_camera.lookAt(0, 0, 0, 0, 89, 15); }
        else if (key == GLFW_KEY_KP_5) { m_camera.setOrtho(!m_camera.isOrtho()); }

        // Load OBJ
        else if (key == GLFW_KEY_O && (mods & GLFW_MOD_CONTROL)) {
            loadOBJDialog();
        }

        // Mesh ops
        else if (key == GLFW_KEY_W && m_editMode && m_selectedObject) {
            m_selectedObject->mesh().subdivide();
        }
        else if (key == GLFW_KEY_E && m_editMode && m_selectedObject) {
            int fi = getEditFaceUnderCursor();
            if (fi >= 0) m_selectedObject->mesh().extrude(fi, 0.3f);
        }
        else if (key == GLFW_KEY_L && m_editMode && m_selectedObject) {
            int v0, v1;
            if (m_selectedObject->mesh().getSelectedEdge(v0, v1)) {
                m_selectedObject->mesh().loopCut(v0, v1);
            }
        }
        else if (key == GLFW_KEY_B && m_editMode && m_selectedObject) {
            int v[4] = {-1,-1,-1,-1};
            int cnt = 0;
            for (int i = 0; i < m_selectedObject->mesh().vertexCount() && cnt < 4; i++) {
                if (m_selectedObject->mesh().isSelected(i)) v[cnt++] = i;
            }
            if (cnt == 4)
                m_selectedObject->mesh().bridge(v[0], v[1], v[2], v[3]);
        }

        else if (key == GLFW_KEY_F12) {
            m_screenshotPending = true;
        }
        else if (key == GLFW_KEY_ESCAPE) {
            if (m_mouseCaptured) {
                m_mouseCaptured = false;
                glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                setKeyboardFocus(false);
            }
        }
    }
    return m_keyboardFocus;
}

void ViewportPanel::loadOBJDialog() {
    std::string path = openFileDialog();
    if (path.empty()) return;

    // Use filename without extension as DB name
    size_t sep = path.find_last_of("\\/");
    std::string name = (sep != std::string::npos) ? path.substr(sep + 1) : path;
    size_t dot = name.rfind('.');
    if (dot != std::string::npos) name = name.substr(0, dot);

    if (!m_db.loadFile(name, path)) {
        fprintf(stderr, "Fehler beim Laden von %s\n", path.c_str());
        return;
    }

    // Spawn new object with this mesh
    char nbuf[32]; snprintf(nbuf, sizeof(nbuf), "Mesh_%d", (int)m_objects.size());
    m_objects.emplace_back(0.0f, 0.0f, 0.0f, nbuf);
    SceneObject& obj = m_objects.back();
    if (MeshData* md = m_db.find(name)) {
        obj.mesh() = *md;
    }
    selectObject(&obj);
    printf("OBJ geladen: %s (%d verts)\n", name.c_str(), obj.mesh().vertexCount());
}

int ViewportPanel::addObject(float x, float y, float z) {
    char nbuf[32]; snprintf(nbuf, sizeof(nbuf), "Object_%d", (int)m_objects.size());
    m_objects.emplace_back(x, y, z, nbuf);
    return (int)m_objects.size() - 1;
}

void ViewportPanel::selectObjectByIndex(int i) {
    if (i >= 0 && i < (int)m_objects.size())
        selectObject(&m_objects[i]);
    else
        selectObject(nullptr);
}

void ViewportPanel::removeObject(int i) {
    if (i < 0 || i >= (int)m_objects.size()) return;
    if (m_selectedObject == &m_objects[i]) selectObject(nullptr);
    m_objects.erase(m_objects.begin() + i);
    // adjust selection index after removal
    if (i < (int)m_objects.size()) selectObject(&m_objects[i]);
}

void ViewportPanel::setEditMode(bool on) {
    m_editMode = on;
    if (m_selectedObject) m_selectedObject->mesh().clearSelection();
}

void ViewportPanel::loadOBJFile(const std::string& path) {
    size_t sep = path.find_last_of("\\/");
    std::string name = (sep != std::string::npos) ? path.substr(sep + 1) : path;
    size_t dot = name.rfind('.');
    if (dot != std::string::npos) name = name.substr(0, dot);

    if (!m_db.loadFile(name, path)) {
        fprintf(stderr, "Fehler beim Laden von %s\n", path.c_str());
        return;
    }
    int idx = addObject(0, 0, 0);
    if (MeshData* md = m_db.find(name)) {
        m_objects[idx].mesh() = *md;
    }
    selectObjectByIndex(idx);
    printf("OBJ geladen: %s (%d verts)\n", name.c_str(), m_objects[idx].mesh().vertexCount());
}

void ViewportPanel::loadOBJDirectory(const std::string& dir) {
    if (!fs::is_directory(dir)) {
        printf("OBJ-Verzeichnis %s nicht gefunden\n", dir.c_str());
        return;
    }
    int count = 0;
    for (auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() != ".obj") continue;
        std::string name = entry.path().stem().string();
        if (m_db.loadFile(name, entry.path().string())) count++;
    }
    printf("OBJ-DB: %d Dateien aus %s geladen\n", count, dir.c_str());
}
