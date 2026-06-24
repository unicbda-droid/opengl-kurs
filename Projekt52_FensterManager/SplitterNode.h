#pragma once
#include "Panel.h"

class SplitterNode : public Panel {
public:
    SplitterNode(GLFWwindow* window, bool horizontal, float splitRatio = 0.5f);
    ~SplitterNode() override;

    void setChild1(Panel* child);
    void setChild2(Panel* child);

    void update(float dt) override;
    void setRect(int x, int y, int w, int h) override;
    void render() override;

    bool onMouseButton(int button, int action, int mods) override;
    bool onCursorPos(double x, double y) override;

    Panel* findPanelAt(int x, int y) override;
    SplitterNode* findGrabberAt(int x, int y) override;
    Panel* findCapturedPanel() override;
    Panel* findKeyboardFocusedPanel() override;
    void forAllPanels(const std::function<void(Panel*)>& cb) override;

    Rect getGrabberRect() const;
    bool isHorizontal() const { return m_horizontal; }
    Panel* getChild1() const { return m_child1; }
    Panel* getChild2() const { return m_child2; }

    static constexpr int GRABBER_SIZE = 6;

private:
    bool m_horizontal;
    float m_ratio;
    Panel* m_child1 = nullptr;
    Panel* m_child2 = nullptr;
    bool m_dragging = false;
};
