#pragma once
#include "MeshData.h"

enum class ViewMode {
    Solid,
    Wireframe,
    Normal,
    Texture
};

class SceneObject {
public:
    SceneObject(float x, float y, float z, const std::string& name = "Object");

    void draw(ViewMode mode, const std::vector<SceneObject>* allObjs = nullptr);

    void setPosition(float x, float y, float z) { m_x = x; m_y = y; m_z = z; }
    void getPosition(float& x, float& y, float& z) const { x = m_x; y = m_y; z = m_z; }
    void getWorldPosition(float& x, float& y, float& z, const std::vector<SceneObject>* allObjs = nullptr) const;

    void setRotation(float yaw, float pitch, float roll) { m_ry = yaw; m_rx = pitch; m_rz = roll; }
    void getRotation(float& yaw, float& pitch, float& roll) const { yaw = m_ry; pitch = m_rx; roll = m_rz; }

    void setScale(float s) { m_scale = s; }
    float getScale() const { return m_scale; }

    bool isSelected() const { return m_selected; }
    void setSelected(bool s) { m_selected = s; }

    MeshData& mesh() { return m_mesh; }
    const MeshData& mesh() const { return m_mesh; }

    void setParent(int idx) { m_parentIndex = idx; }
    int getParentIndex() const { return m_parentIndex; }

    void setName(const std::string& n) { m_name = n; }
    const std::string& getName() const { return m_name; }

private:
    float m_x = 0.0f, m_y = 0.0f, m_z = 0.0f;
    float m_rx = 0.0f, m_ry = 0.0f, m_rz = 0.0f;
    float m_scale = 1.0f;
    bool m_selected = false;
    MeshData m_mesh;
    int m_parentIndex = -1;
    std::string m_name;

    void applyTransformChain(const std::vector<SceneObject>& allObjs) const;
};
