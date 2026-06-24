#pragma once
#include "Panel.h"

class EmptyPanel : public Panel {
public:
    EmptyPanel(GLFWwindow* window, const std::string& label = "Properties");
    void render() override;

private:
    std::string m_label;
};
