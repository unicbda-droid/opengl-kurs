#pragma once
#include "Panel.h"
#include <vector>

class MenuPanel : public Panel {
public:
    MenuPanel(GLFWwindow* window);
    void render() override;
    bool onMouseButton(int button, int action, int mods) override;
    bool onCursorPos(double x, double y) override;

private:
    struct MenuItem {
        std::string label;
        Rect rect;
    };
    std::vector<MenuItem> m_items;
    int m_hovered = -1;
};
