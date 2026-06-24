#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <string>
#include <vector>

GLuint textureID;
float cnt = 0.0f;

// Generiert eine prozedurale 128x128 Zeichentabelle (Font-Sheet)
// 16x16 Kacheln. Wir zeichnen symbolische weiße Kreuze/Muster für die Zeichen
void createFontSheet() {
    std::vector<GLubyte> img(128*128*3, 0);
    for(int cx=0; cx<16; cx++) {
        for(int cy=0; cy<16; cy++) {
            for(int i=2; i<6; i++) {
                int px = cx*8 + i; int py = cy*8 + 4;
                int idx1 = (py*128 + px)*3;
                img[idx1]=255; img[idx1+1]=255; img[idx1+2]=255;
                int px2 = cx*8 + 4; int py2 = cy*8 + i;
                int idx2 = (py2*128 + px2)*3;
                img[idx2]=255; img[idx2+1]=255; img[idx2+2]=255;
            }
        }
    }
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
}

// Zeichnet Text, indem die Texturkoordinaten passend verschoben werden
void buildFontText(const std::string& text, float spacing) {
    glBegin(GL_QUADS);
    for(size_t i=0; i<text.length(); i++) {
        char c = text[i];
        // Berechne die Spalte und Zeile auf dem 16x16 Kachel-Sheet
        float cx = float(c % 16) / 16.0f;
        float cy = float(c / 16) / 16.0f;
        float size = 1.0f / 16.0f;

        // Schiebe das Quad für jeden Buchstaben ein Stück nach rechts
        float x_offset = float(i) * spacing;

        glTexCoord2f(cx, cy);               glVertex2f(x_offset,        0.0f);
        glTexCoord2f(cx + size, cy);        glVertex2f(x_offset + 1.0f, 0.0f);
        glTexCoord2f(cx + size, cy + size); glVertex2f(x_offset + 1.0f, 1.0f);
        glTexCoord2f(cx, cy + size);        glVertex2f(x_offset,        1.0f);
    }
    glEnd();
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 17 - Font-Sheets", NULL, NULL);
    glfwMakeContextCurrent(w);
    glEnable(GL_TEXTURE_2D); createFontSheet();
    
    // Orthogonaler 2D-Modus für UI/Text-Overlays
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0.0, 800.0, 0.0, 600.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f); glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();

        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Text 1: Statisch oben links
        glTranslatef(50.0f, 500.0f, 0.0f);
        glScalef(24.0f, 24.0f, 1.0f); // Skaliert die Buchstaben-Größe
        glColor3f(0.0f, 1.0f, 0.5f); 
        buildFontText("NEHE OPENGL", 0.75f);

        // Text 2: Pulsierend in der Mitte
        glLoadIdentity();
        glTranslatef(150.0f + 50.0f * std::cos(cnt), 300.0f, 0.0f);
        glScalef(32.0f + 10.0f * std::sin(cnt), 32.0f + 10.0f * std::sin(cnt), 1.0f);
        glColor3f(1.0f, 0.5f, 0.0f);
        buildFontText("SPEEDRUN", 0.7f);

        cnt += 0.05f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}