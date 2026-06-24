#pragma once
#include <GL/glew.h>

enum class GizmoMode {
    Move,
    Rotate,
    Scale
};

class Gizmo {
public:
    Gizmo() = default;

    void render(float objX, float objY, float objZ, GizmoMode mode);
    int hitTest(float mx, float my, int viewport[4], float objX, float objY, float objZ, GizmoMode mode);

    int getActiveAxis() const { return m_activeAxis; }
    void setActiveAxis(int a) { m_activeAxis = a; }
    bool isDragging() const { return m_dragging; }
    void setDragging(bool d) { m_dragging = d; }

    void getDragDelta(float mx, float my, float objX, float objY, float objZ,
                      float& dx, float& dy, float& dz);

    void setStartDrag(float mx, float my) { m_startMX = mx; m_startMY = my; }
    void setPos(float mx, float my) { m_lastMX = mx; m_lastMY = my; }

    void setAxis(int a) { m_highlightAxis = a; }
    int getAxis() const { return m_highlightAxis; }

private:
    int m_activeAxis = -1;
    int m_highlightAxis = -1;
    bool m_dragging = false;
    float m_startMX = 0, m_startMY = 0;
    float m_lastMX = 0, m_lastMY = 0;

    static constexpr float AXIS_LEN = 2.5f;
    static constexpr float AXIS_SIZE = 0.06f;

    void drawArrow(float ex, float ey, float ez, float r, float g, float b);
    void drawRing(float nx, float ny, float nz, float r, float g, float b);
    void drawScaleBox(float ex, float ey, float ez, float r, float g, float b);
    void drawCone(float ex, float ey, float ez, float r, float g, float b);
    float axisDist(int ax, float mx, float my, int vp[4], float ox, float oy, float oz);
};
