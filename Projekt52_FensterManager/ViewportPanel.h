#pragma once
#include "Panel.h"
#include "Camera.h"
#include "Gizmo.h"
#include "SceneObject.h"
#include "MeshDatabase.h"
#include <vector>
#include <string>

class ViewportPanel : public Panel {
public:
    ViewportPanel(GLFWwindow* window);

    void update(float dt) override;
    void render() override;

    bool onMouseButton(int button, int action, int mods) override;
    bool onCursorPos(double x, double y) override;
    bool onScroll(double dx, double dy) override;
    bool onKey(int key, int scancode, int action, int mods) override;
    bool isCaptured() const override { return m_mouseCaptured || m_gizmo.isDragging(); }
    int objectCount() const { return (int)m_objects.size(); }
    SceneObject* getSelectedObject() { return m_selectedObject; }
    SceneObject* getObject(int i) { return (i >= 0 && i < (int)m_objects.size()) ? &m_objects[i] : nullptr; }
    int addObject(float x, float y, float z);
    void selectObjectByIndex(int i);
    void removeObject(int i);
    void setViewMode(ViewMode mode) { m_viewMode = mode; }
    void setEditMode(bool on);
    bool isEditMode() const { return m_editMode; }
    void loadOBJFile(const std::string& path);
    MeshDatabase& getDB() { return m_db; }

private:
    Camera m_camera;
    Gizmo m_gizmo;
    GizmoMode m_gizmoMode = GizmoMode::Move;
    ViewMode m_viewMode = ViewMode::Solid;
    std::vector<SceneObject> m_objects;
    SceneObject* m_selectedObject = nullptr;

    bool m_editMode = false;
    bool m_mouseCaptured = false;
    bool m_leftDown = false;
    bool m_rightDown = false;
    bool m_shiftDown = false;
    double m_lastMX = 0, m_lastMY = 0;

    struct { int x, y; bool pending = false; } m_pick;
    bool m_screenshotPending = false;

    MeshDatabase m_db;

    void drawGrid();
    void drawAxes();
    void drawScene();
    void doPick();
    void selectObject(SceneObject* obj);
    void doVertexPick();
    int getEditFaceUnderCursor();
    void loadOBJDialog();
    void loadOBJDirectory(const std::string& dir);
    void saveScreenshot();
};
