#pragma once
#include "Panel.h"
#include "BitmapFont.h"
#include <string>
#include <vector>
#include <deque>

class ViewportPanel;

class ConsolePanel : public Panel {
public:
    ConsolePanel(GLFWwindow* window, ViewportPanel* viewport);

    void render() override;
    bool onMouseButton(int button, int action, int mods) override;
    bool onKey(int key, int scancode, int action, int mods) override;
    bool onChar(unsigned int codepoint) override;
    bool isCaptured() const override { return false; }
    // hasKeyboardFocus inherited from Panel

    void print(const std::string& text);

private:
    ViewportPanel* m_viewport;
    BitmapFont m_font;

    std::string m_input;
    std::deque<std::string> m_output;
    std::vector<std::string> m_history;
    int m_historyPos = -1;
    int m_scrollOffset = 0;
    int m_maxLines = 100;

    void execute(const std::string& cmd);
    void drawInputLine();
    void drawOutput();
    std::vector<std::string> splitArgs(const std::string& text);
};
