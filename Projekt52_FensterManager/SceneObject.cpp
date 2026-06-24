#include "SceneObject.h"
#include <GL/glew.h>

SceneObject::SceneObject(float x, float y, float z, const std::string& name)
    : m_x(x), m_y(y), m_z(z), m_name(name) {
    m_mesh = MeshData::createCube();
}

void SceneObject::applyTransformChain(const std::vector<SceneObject>& allObjs) const {
    if (m_parentIndex >= 0) {
        allObjs[m_parentIndex].applyTransformChain(allObjs);
    }
    glTranslatef(m_x, m_y, m_z);
    glRotatef(m_rx, 1, 0, 0);
    glRotatef(m_ry, 0, 1, 0);
    glRotatef(m_rz, 0, 0, 1);
    glScalef(m_scale, m_scale, m_scale);
}

void SceneObject::getWorldPosition(float& x, float& y, float& z,
                                    const std::vector<SceneObject>* allObjs) const {
    if (allObjs && m_parentIndex >= 0) {
        (*allObjs)[m_parentIndex].getWorldPosition(x, y, z, allObjs);
        x += m_x; y += m_y; z += m_z;
    } else {
        x = m_x; y = m_y; z = m_z;
    }
}

void SceneObject::draw(ViewMode mode, const std::vector<SceneObject>* allObjs) {
    glPushMatrix();

    if (allObjs) {
        applyTransformChain(*allObjs);
    } else {
        glTranslatef(m_x, m_y, m_z);
        glRotatef(m_rx, 1, 0, 0);
        glRotatef(m_ry, 0, 1, 0);
        glRotatef(m_rz, 0, 0, 1);
        glScalef(m_scale, m_scale, m_scale);
    }

    glEnable(GL_DEPTH_TEST);

    switch (mode) {
        case ViewMode::Solid:     m_mesh.drawSolid(); break;
        case ViewMode::Wireframe: m_mesh.drawWireframe(); break;
        case ViewMode::Normal:    m_mesh.drawNormal(); break;
        case ViewMode::Texture:   m_mesh.drawTexture(); break;
    }

    if (mode != ViewMode::Wireframe) {
        m_mesh.drawVertices(6.0f);
    }

    m_mesh.drawHighlightedEdges();
    m_mesh.drawHighlightedFace();

    if (m_selected && mode != ViewMode::Wireframe && mode != ViewMode::Texture) {
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(1, 1, 0);
        glLineWidth(2.0f);
        m_mesh.drawSolid();
        glLineWidth(1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);
    }

    glDisable(GL_DEPTH_TEST);
    glPopMatrix();
}
