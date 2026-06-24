#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <cmath>

float rX = 0.0f;
const int BLIT_W = 200;
const int BLIT_H = 150;
std::vector<GLubyte> pixelData(BLIT_W * BLIT_H * 3);

// Generiert ein dynamisches UI-Pixelmuster im Arbeitsspeicher
void updateBlitData(float frame) {
    for (int y = 0; y < BLIT_H; y++) {
        for (int x = 0; x < BLIT_W; x++) {
            int idx = (y * BLIT_W + x) * 3;
            // Ein psychedelisches, wanderndes Streifenmuster für das "UI-Fenster"
            pixelData[idx]     = (GLubyte)(127.0f + 127.0f * std::sin((x + frame) * 0.05f));
            pixelData[idx + 1] = (GLubyte)(127.0f + 127.0f * std::cos((y + frame) * 0.05f));
            pixelData[idx + 2] = 255;
        }
    }
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 29 - Raw Blitting", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    float frameCounter = 0.0f;

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 1. Eine normale 3D-Szene im Hintergrund zeichnen
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -3.0f);
        glRotatef(rX, 1.0f, 1.0f, 1.0f);
        
        glBegin(GL_QUADS);
            glColor3f(0.5f, 0.0f, 0.5f); glVertex3f(-0.7f, -0.7f, 0.0f); 
            glColor3f(0.0f, 0.5f, 0.5f); glVertex3f( 0.7f, -0.7f, 0.0f);
            glColor3f(0.5f, 0.5f, 0.0f); glVertex3f( 0.7f,  0.7f, 0.0f);
            glColor3f(0.0f, 0.0f, 0.5f); glVertex3f(-0.7f,  0.7f, 0.0f);
        glEnd();

        // 2. DIE REINE PIXEL-OPERATION (BLITTING)
        // Wir schalten den Tiefentest aus, damit das UI IMMER im Vordergrund steht
        glDisable(GL_DEPTH_TEST);
        
        // Wir aktualisieren die Pixel im RAM
        updateBlitData(frameCounter);
        
        // Raster-Position auf dem Bildschirm setzen (unten links ist 0,0 in OpenGL)
        // Wir "blitten" das Fenster oben links hin
        glRasterPos2f(-0.95f, 0.5f);
        
        // Kopiert die rohen Speicherbits direkt auf den Monitor
        glDrawPixels(BLIT_W, BLIT_H, GL_RGB, GL_UNSIGNED_BYTE, pixelData.data());
        
        glEnable(GL_DEPTH_TEST);

        rX += 0.8f; frameCounter += 1.0f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}