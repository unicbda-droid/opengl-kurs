#pragma once
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Rect.h"

class SplitterNode;

class Panel {
public:
    Panel(const std::string& name, GLFWwindow* window);
    virtual ~Panel() = default;

    virtual void update(float dt) {}
    virtual void setRect(int x, int y, int w, int h);
    virtual void render() = 0;

    virtual bool onMouseButton(int button, int action, int mods) { return false; }
    virtual bool onCursorPos(double x, double y) { return false; }
    virtual bool onScroll(double dx, double dy) { return false; }
    virtual bool onKey(int key, int scancode, int action, int mods) { return false; }

    bool contains(int px, int py) const { return m_rect.contains(px, py); }
    const Rect& getRect() const { return m_rect; }
    const std::string& getName() const { return m_name; }

    virtual Panel* findPanelAt(int x, int y);
    virtual SplitterNode* findGrabberAt(int x, int y);
    virtual bool isCaptured() const { return false; }
    virtual Panel* findCapturedPanel();
    virtual bool hasKeyboardFocus() const { return false; }
    virtual Panel* findKeyboardFocusedPanel();

protected:
    void setupViewport();

    Rect m_rect = {};
    std::string m_name;
    GLFWwindow* m_window = nullptr;
};
