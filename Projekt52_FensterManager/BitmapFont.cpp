#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "BitmapFont.h"

BitmapFont::BitmapFont() {
    HDC hdc = wglGetCurrentDC();
    if (!hdc) return;

    m_base = glGenLists(96);
    if (!m_base) return;

    HFONT font = CreateFontA(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");

    if (!font) {
        font = CreateFontA(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, "Lucida Console");
    }

    if (!font) return;

    HFONT old = (HFONT)SelectObject(hdc, font);
    wglUseFontBitmaps(hdc, 32, 96, m_base);
    SelectObject(hdc, old);
    DeleteObject(font);
}

BitmapFont::~BitmapFont() {
    if (m_base) glDeleteLists(m_base, 96);
}

void BitmapFont::print(int x, int y, const std::string& text, float r, float g, float b) {
    if (!m_base || text.empty()) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, m_winW, 0, m_winH, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(r, g, b);
    glRasterPos2i(x, y);

    glListBase(m_base);
    glCallLists((GLsizei)text.size(), GL_UNSIGNED_BYTE, text.c_str());

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
}
