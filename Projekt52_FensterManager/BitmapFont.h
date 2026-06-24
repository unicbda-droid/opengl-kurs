#pragma once
#include <GL/glew.h>
#include <string>

class BitmapFont {
public:
    BitmapFont();
    ~BitmapFont();
    void print(int x, int y, const std::string& text, float r = 1, float g = 1, float b = 1);
    void setWindowSize(int w, int h) { m_winW = w; m_winH = h; }
    int charWidth() const { return 8; }
    int charHeight() const { return 14; }
    int textWidth(const std::string& text) const { return (int)text.size() * 8; }
private:
    GLuint m_base = 0;
    int m_winW = 1280, m_winH = 720;
};
