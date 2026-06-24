#include "Gizmo.h"
#include <GL/glew.h>
#include <cmath>
#include <cfloat>

static const float PI = 3.14159265f;

static bool project(double ox, double oy, double oz,
                    const double mod[16], const double proj[16],
                    const int vp[4], double& wx, double& wy, double& wz) {
    double in[4] = {ox, oy, oz, 1.0}, out[4];
    for (int i = 0; i < 4; i++)
        out[i] = mod[i*4]*in[0] + mod[i*4+1]*in[1] + mod[i*4+2]*in[2] + mod[i*4+3]*in[3];
    for (int i = 0; i < 4; i++)
        in[i] = proj[i*4]*out[0] + proj[i*4+1]*out[1] + proj[i*4+2]*out[2] + proj[i*4+3]*out[3];
    if (in[3] == 0.0) return false;
    in[0] /= in[3]; in[1] /= in[3]; in[2] /= in[3];
    wx = vp[0] + (1.0 + in[0]) * vp[2] / 2.0;
    wy = vp[1] + (1.0 + in[1]) * vp[3] / 2.0;
    wz = (1.0 + in[2]) / 2.0;
    return true;
}

void Gizmo::render(float objX, float objY, float objZ, GizmoMode mode) {
    glPushMatrix();
    glTranslatef(objX, objY, objZ);

    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);

    if (mode == GizmoMode::Move) {
        drawArrow(AXIS_LEN,0,0, 1,0,0);
        drawArrow(0,AXIS_LEN,0, 0,1,0);
        drawArrow(0,0,AXIS_LEN, 0,0,1);
    } else if (mode == GizmoMode::Rotate) {
        drawRing(0,0,1, 1,0,0);
        drawRing(0,1,0, 0,1,0);
        drawRing(1,0,0, 0,0,1);
    } else if (mode == GizmoMode::Scale) {
        drawScaleBox(AXIS_LEN,0,0, 1,0,0);
        drawScaleBox(0,AXIS_LEN,0, 0,1,0);
        drawScaleBox(0,0,AXIS_LEN, 0,0,1);
    }

    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
}

void Gizmo::drawArrow(float ex, float ey, float ez, float r, float g, float b) {
    bool hl = m_highlightAxis >= 0 &&
              ((ex > 0 && m_highlightAxis == 0) ||
               (ey > 0 && m_highlightAxis == 1) ||
               (ez > 0 && m_highlightAxis == 2));
    bool act = m_activeAxis >= 0 &&
               ((ex > 0 && m_activeAxis == 0) ||
                (ey > 0 && m_activeAxis == 1) ||
                (ez > 0 && m_activeAxis == 2));
    float br = hl || act ? 1.0f : r * 0.6f;
    float bg = hl || act ? 1.0f : g * 0.6f;
    float bb = hl || act ? 1.0f : b * 0.6f;

    glColor3f(br, bg, bb);
    glBegin(GL_LINES);
    glVertex3f(0,0,0); glVertex3f(ex, ey, ez);
    glEnd();

    float len = std::sqrt(ex*ex + ey*ey + ez*ez);
    float nx = ex/len, ny = ey/len, nz = ez/len;
    glPushMatrix();
    glTranslatef(ex, ey, ez);
    glRotatef(-std::atan2(nx, nz) * 180.0f / PI + 180.0f, 0, 1, 0);
    float pitch = std::asin(ny) * 180.0f / PI;
    glRotatef(pitch, 1, 0, 0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.3f, 0, 0);
    for (int i = 0; i <= 12; i++) {
        float a = (float)i / 12.0f * 2.0f * PI;
        glVertex3f(0, std::sin(a) * 0.12f, std::cos(a) * 0.12f);
    }
    glEnd();
    glPopMatrix();
}

void Gizmo::drawRing(float nx, float ny, float nz, float r, float g, float b) {
    bool hl = m_highlightAxis >= 0 &&
              ((nx > 0 && m_highlightAxis == 0) ||
               (ny > 0 && m_highlightAxis == 1) ||
               (nz > 0 && m_highlightAxis == 2));
    float br = hl ? 1.0f : r * 0.6f;
    float bg = hl ? 1.0f : g * 0.6f;
    float bb = hl ? 1.0f : b * 0.6f;

    glColor3f(br, bg, bb);

    float ux = 0, uy = 0, uz = 0;
    if (nx > 0.5f) { uy = 1; }
    else if (ny > 0.5f) { uz = 1; }
    else { ux = 1; }

    float vx = ny*uz - nz*uy;
    float vy = nz*ux - nx*uz;
    float vz = nx*uy - ny*ux;
    float vl = std::sqrt(vx*vx + vy*vy + vz*vz);
    if (vl > 0) { vx /= vl; vy /= vl; vz /= vl; }

    float wx = vy*nz - vz*ny;
    float wy = vz*nx - vx*nz;
    float wz = vx*ny - vy*nx;

    float radius = 2.0f;
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 32; i++) {
        float a = (float)i / 32.0f * 2.0f * PI;
        float ca = std::cos(a), sa = std::sin(a);
        float px = radius * (ca * vx + sa * wx);
        float py = radius * (ca * vy + sa * wy);
        float pz = radius * (ca * vz + sa * wz);
        glVertex3f(px, py, pz);
    }
    glEnd();
}

void Gizmo::drawScaleBox(float ex, float ey, float ez, float r, float g, float b) {
    bool hl = m_highlightAxis >= 0 &&
              ((ex > 0 && m_highlightAxis == 0) ||
               (ey > 0 && m_highlightAxis == 1) ||
               (ez > 0 && m_highlightAxis == 2));
    float br = hl ? 1.0f : r * 0.6f;
    float bg = hl ? 1.0f : g * 0.6f;
    float bb = hl ? 1.0f : b * 0.6f;

    glColor3f(br, bg, bb);
    glBegin(GL_LINES);
    glVertex3f(0,0,0); glVertex3f(ex, ey, ez);
    glEnd();

    float s = 0.15f;
    glPushMatrix();
    glTranslatef(ex, ey, ez);
    glBegin(GL_QUADS);
    glVertex3f(-s,-s,s); glVertex3f(s,-s,s); glVertex3f(s,s,s); glVertex3f(-s,s,s);
    glVertex3f(s,-s,-s); glVertex3f(-s,-s,-s); glVertex3f(-s,s,-s); glVertex3f(s,s,-s);
    glVertex3f(-s,s,s); glVertex3f(s,s,s); glVertex3f(s,s,-s); glVertex3f(-s,s,-s);
    glVertex3f(-s,-s,-s); glVertex3f(s,-s,-s); glVertex3f(s,-s,s); glVertex3f(-s,-s,s);
    glVertex3f(s,-s,s); glVertex3f(s,-s,-s); glVertex3f(s,s,-s); glVertex3f(s,s,s);
    glVertex3f(-s,-s,-s); glVertex3f(-s,-s,s); glVertex3f(-s,s,s); glVertex3f(-s,s,-s);
    glEnd();
    glPopMatrix();
}

int Gizmo::hitTest(float mx, float my, int viewport[4],
                   float objX, float objY, float objZ, GizmoMode mode) {
    float best = 0.02f;
    int bestAxis = -1;
    for (int a = 0; a < 3; a++) {
        float d = axisDist(a, mx, my, viewport, objX, objY, objZ);
        if (d < best) { best = d; bestAxis = a; }
    }
    return bestAxis;
}

float Gizmo::axisDist(int ax, float mx, float my, int vp[4],
                      float ox, float oy, float oz) {
    double p0[3], p1[3];
    float ex = 0, ey = 0, ez = 0;
    if (ax == 0) ex = AXIS_LEN;
    else if (ax == 1) ey = AXIS_LEN;
    else ez = AXIS_LEN;

    GLdouble mod[16], proj[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, mod);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);

    project(ox, oy, oz, mod, proj, vp, p0[0], p0[1], p0[2]);
    project(ox+ex, oy+ey, oz+ez, mod, proj, vp, p1[0], p1[1], p1[2]);

    double dx = p1[0] - p0[0], dy = p1[1] - p0[1];
    double len = std::sqrt(dx*dx + dy*dy);
    if (len < 1.0) return FLT_MAX;

    double t = ((mx - p0[0])*dx + (my - p0[1])*dy) / (len*len);
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    double cx = p0[0] + t*dx, cy = p0[1] + t*dy;
    double ddx = mx - cx, ddy = my - cy;
    return (float)std::sqrt(ddx*ddx + ddy*ddy);
}

void Gizmo::getDragDelta(float mx, float my, float objX, float objY, float objZ,
                         float& dx, float& dy, float& dz) {
    dx = dy = dz = 0;
    if (m_activeAxis < 0) return;

    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    double mod[16], proj[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, mod);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);

    double s[3], e[3];
    float ex=0, ey=0, ez=0;
    if (m_activeAxis == 0) ex = 1;
    else if (m_activeAxis == 1) ey = 1;
    else ez = 1;

    project(objX, objY, objZ, mod, proj, vp, s[0], s[1], s[2]);
    project(objX+ex, objY+ey, objZ+ez, mod, proj, vp, e[0], e[1], e[2]);

    double adx = e[0]-s[0], ady = e[1]-s[1];
    double alen = std::sqrt(adx*adx + ady*ady);
    if (alen < 0.001) return;

    double dt = ((mx - m_lastMX)*adx + (my - m_lastMY)*ady) / alen;
    if (m_activeAxis == 0) dx = (float)dt * 0.05f;
    else if (m_activeAxis == 1) dy = (float)dt * 0.05f;
    else dz = (float)dt * 0.05f;

    m_lastMX = mx;
    m_lastMY = my;
}
