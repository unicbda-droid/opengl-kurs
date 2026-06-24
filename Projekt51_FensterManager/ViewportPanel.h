#pragma once
#include "Panel.h"
#include "Camera.h"
#include "Gizmo.h"
#include "SceneObject.h"
#include <vector>

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
    bool hasKeyboardFocus() const override { return m_hasFocus; }

    Camera& getCamera() { return m_camera; }
    SceneObject* getSelectedObject() { return m_selectedObject; }
    ViewMode getViewMode() const { return m_viewMode; }

private:
    Camera m_camera;
    Gizmo m_gizmo;
    GizmoMode m_gizmoMode = GizmoMode::Move;
    ViewMode m_viewMode = ViewMode::Solid;
    std::vector<SceneObject> m_objects;
    SceneObject* m_selectedObject = nullptr;

    bool m_mouseCaptured = false;
    bool m_hasFocus = false;
    bool m_leftDown = false;
    bool m_rightDown = false;
    double m_lastMX = 0, m_lastMY = 0;

    struct { int x, y; bool pending = false; } m_pick;

    void drawGrid();
    void drawAxes();
    void drawScene();
    void doPick();
    void selectObject(SceneObject* obj);
};
