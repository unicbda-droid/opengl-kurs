#include "SceneObject.h"
#include <GL/glew.h>
#include <cmath>

static const float PI = 3.14159265f;

void SceneObject::draw(ViewMode mode) {
    glPushMatrix();
    glTranslatef(m_x, m_y, m_z);
    glRotatef(m_rx, 1, 0, 0);
    glRotatef(m_ry, 0, 1, 0);
    glRotatef(m_rz, 0, 0, 1);
    glScalef(m_scale, m_scale, m_scale);

    switch (mode) {
        case ViewMode::Solid:     drawSolid(); break;
        case ViewMode::Wireframe: drawWireframe(); break;
        case ViewMode::Normal:    drawNormal(); break;
        case ViewMode::Texture:   drawTexture(); break;
    }

    glPopMatrix();
}

void SceneObject::drawSolid() {
    glColor3f(0.8f, 0.3f, 0.2f);
    glBegin(GL_QUADS);
    glNormal3f(0,0,1); glVertex3f(-1,-1,1); glVertex3f(1,-1,1); glVertex3f(1,1,1); glVertex3f(-1,1,1);
    glNormal3f(0,0,-1); glVertex3f(1,-1,-1); glVertex3f(-1,-1,-1); glVertex3f(-1,1,-1); glVertex3f(1,1,-1);
    glNormal3f(0,1,0); glVertex3f(-1,1,1); glVertex3f(1,1,1); glVertex3f(1,1,-1); glVertex3f(-1,1,-1);
    glNormal3f(0,-1,0); glVertex3f(-1,-1,-1); glVertex3f(1,-1,-1); glVertex3f(1,-1,1); glVertex3f(-1,-1,1);
    glNormal3f(1,0,0); glVertex3f(1,-1,1); glVertex3f(1,-1,-1); glVertex3f(1,1,-1); glVertex3f(1,1,1);
    glNormal3f(-1,0,0); glVertex3f(-1,-1,-1); glVertex3f(-1,-1,1); glVertex3f(-1,1,1); glVertex3f(-1,1,-1);
    glEnd();

    if (m_selected) {
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(1, 1, 0);
        glLineWidth(2.0f);
        glBegin(GL_QUADS);
        glVertex3f(-1,-1,1); glVertex3f(1,-1,1); glVertex3f(1,1,1); glVertex3f(-1,1,1);
        glVertex3f(1,-1,-1); glVertex3f(-1,-1,-1); glVertex3f(-1,1,-1); glVertex3f(1,1,-1);
        glVertex3f(-1,1,1); glVertex3f(1,1,1); glVertex3f(1,1,-1); glVertex3f(-1,1,-1);
        glVertex3f(-1,-1,-1); glVertex3f(1,-1,-1); glVertex3f(1,-1,1); glVertex3f(-1,-1,1);
        glVertex3f(1,-1,1); glVertex3f(1,-1,-1); glVertex3f(1,1,-1); glVertex3f(1,1,1);
        glVertex3f(-1,-1,-1); glVertex3f(-1,-1,1); glVertex3f(-1,1,1); glVertex3f(-1,1,-1);
        glEnd();
        glEnd();
        glLineWidth(1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);
    }
}

void SceneObject::drawWireframe() {
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.8f, 0.8f, 0.2f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_QUADS);
    glVertex3f(-1,-1,1); glVertex3f(1,-1,1); glVertex3f(1,1,1); glVertex3f(-1,1,1);
    glVertex3f(1,-1,-1); glVertex3f(-1,-1,-1); glVertex3f(-1,1,-1); glVertex3f(1,1,-1);
    glVertex3f(-1,1,1); glVertex3f(1,1,1); glVertex3f(1,1,-1); glVertex3f(-1,1,-1);
    glVertex3f(-1,-1,-1); glVertex3f(1,-1,-1); glVertex3f(1,-1,1); glVertex3f(-1,-1,1);
    glVertex3f(1,-1,1); glVertex3f(1,-1,-1); glVertex3f(1,1,-1); glVertex3f(1,1,1);
    glVertex3f(-1,-1,-1); glVertex3f(-1,-1,1); glVertex3f(-1,1,1); glVertex3f(-1,1,-1);
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
}

void SceneObject::drawNormal() {
    glBegin(GL_QUADS);
    glColor3f(0,0,1); glNormal3f(0,0,1); glVertex3f(-1,-1,1); glVertex3f(1,-1,1); glVertex3f(1,1,1); glVertex3f(-1,1,1);
    glColor3f(0,0,0.5f); glNormal3f(0,0,-1); glVertex3f(1,-1,-1); glVertex3f(-1,-1,-1); glVertex3f(-1,1,-1); glVertex3f(1,1,-1);
    glColor3f(0,1,0); glNormal3f(0,1,0); glVertex3f(-1,1,1); glVertex3f(1,1,1); glVertex3f(1,1,-1); glVertex3f(-1,1,-1);
    glColor3f(0,0.5f,0); glNormal3f(0,-1,0); glVertex3f(-1,-1,-1); glVertex3f(1,-1,-1); glVertex3f(1,-1,1); glVertex3f(-1,-1,1);
    glColor3f(1,0,0); glNormal3f(1,0,0); glVertex3f(1,-1,1); glVertex3f(1,-1,-1); glVertex3f(1,1,-1); glVertex3f(1,1,1);
    glColor3f(0.5f,0,0); glNormal3f(-1,0,0); glVertex3f(-1,-1,-1); glVertex3f(-1,-1,1); glVertex3f(-1,1,1); glVertex3f(-1,1,-1);
    glEnd();
}

void SceneObject::ensureTexture() {
    if (m_texId != 0) return;
    glGenTextures(1, &m_texId);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    unsigned char data[256*256*3];
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            int idx = (y * 256 + x) * 3;
            bool w = ((x / 32) + (y / 32)) % 2 == 0;
            data[idx+0] = w ? 255 : 60;
            data[idx+1] = w ? 255 : 60;
            data[idx+2] = w ? 255 : 60;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void SceneObject::drawTexture() {
    ensureTexture();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glNormal3f(0,0,1); glVertex3f(-1,-1,1);
    glTexCoord2f(1,0); glNormal3f(0,0,1); glVertex3f(1,-1,1);
    glTexCoord2f(1,1); glNormal3f(0,0,1); glVertex3f(1,1,1);
    glTexCoord2f(0,1); glNormal3f(0,0,1); glVertex3f(-1,1,1);

    glTexCoord2f(1,0); glNormal3f(0,0,-1); glVertex3f(1,-1,-1);
    glTexCoord2f(0,0); glNormal3f(0,0,-1); glVertex3f(-1,-1,-1);
    glTexCoord2f(0,1); glNormal3f(0,0,-1); glVertex3f(-1,1,-1);
    glTexCoord2f(1,1); glNormal3f(0,0,-1); glVertex3f(1,1,-1);

    glTexCoord2f(0,1); glNormal3f(0,1,0); glVertex3f(-1,1,1);
    glTexCoord2f(1,1); glNormal3f(0,1,0); glVertex3f(1,1,1);
    glTexCoord2f(1,0); glNormal3f(0,1,0); glVertex3f(1,1,-1);
    glTexCoord2f(0,0); glNormal3f(0,1,0); glVertex3f(-1,1,-1);

    glTexCoord2f(0,0); glNormal3f(0,-1,0); glVertex3f(-1,-1,-1);
    glTexCoord2f(1,0); glNormal3f(0,-1,0); glVertex3f(1,-1,-1);
    glTexCoord2f(1,1); glNormal3f(0,-1,0); glVertex3f(1,-1,1);
    glTexCoord2f(0,1); glNormal3f(0,-1,0); glVertex3f(-1,-1,1);

    glTexCoord2f(1,0); glNormal3f(1,0,0); glVertex3f(1,-1,1);
    glTexCoord2f(0,0); glNormal3f(1,0,0); glVertex3f(1,-1,-1);
    glTexCoord2f(0,1); glNormal3f(1,0,0); glVertex3f(1,1,-1);
    glTexCoord2f(1,1); glNormal3f(1,0,0); glVertex3f(1,1,1);

    glTexCoord2f(0,0); glNormal3f(-1,0,0); glVertex3f(-1,-1,-1);
    glTexCoord2f(1,0); glNormal3f(-1,0,0); glVertex3f(-1,-1,1);
    glTexCoord2f(1,1); glNormal3f(-1,0,0); glVertex3f(-1,1,1);
    glTexCoord2f(0,1); glNormal3f(-1,0,0); glVertex3f(-1,1,-1);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    if (m_selected) {
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(1, 1, 0);
        glLineWidth(2.0f);
        glBegin(GL_QUADS);
        glVertex3f(-1,-1,1); glVertex3f(1,-1,1); glVertex3f(1,1,1); glVertex3f(-1,1,1);
        glVertex3f(1,-1,-1); glVertex3f(-1,-1,-1); glVertex3f(-1,1,-1); glVertex3f(1,1,-1);
        glVertex3f(-1,1,1); glVertex3f(1,1,1); glVertex3f(1,1,-1); glVertex3f(-1,1,-1);
        glVertex3f(-1,-1,-1); glVertex3f(1,-1,-1); glVertex3f(1,-1,1); glVertex3f(-1,-1,1);
        glVertex3f(1,-1,1); glVertex3f(1,-1,-1); glVertex3f(1,1,-1); glVertex3f(1,1,1);
        glVertex3f(-1,-1,-1); glVertex3f(-1,-1,1); glVertex3f(-1,1,1); glVertex3f(-1,1,-1);
        glEnd();
        glLineWidth(1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);
    }
}
