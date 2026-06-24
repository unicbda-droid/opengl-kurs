#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <cmath>

float rX = 0.0f, rY = 0.0f;
int divs = 15; // Aufloesung der Kurve (wie glatt sie gerendert wird)

// 16 Kontrollpunkte fuer das Bezier-Feld (3D-Gitter)
GLfloat ctrlPoints[4][4][3] = {
    { {-1.5f, -1.5f,  4.0f}, {-0.5f, -1.5f,  2.0f}, { 0.5f, -1.5f, -1.0f}, { 1.5f, -1.5f,  2.0f} },
    { {-1.5f, -0.5f,  1.0f}, {-0.5f,  1.5f,  3.0f}, { 0.5f,  2.5f,  2.0f}, { 1.5f, -0.5f, -1.0f} },
    { {-1.5f,  0.5f,  4.0f}, {-0.5f, -1.5f,  2.0f}, { 0.5f,  1.5f, -3.0f}, { 1.5f,  0.5f,  1.0f} },
    { {-1.5f,  1.5f, -2.0f}, {-0.5f,  1.5f, -2.0f}, { 0.5f,  1.5f,  0.0f}, { 1.5f,  1.5f, -1.0f} }
};

// Hilfsfunktion zur Berechnung eines Punktes auf der Bezier-Kurve
std::vector<float> bezierCalc(float u, float v) {
    std::vector<float> p(3, 0.0f);
    float bu[4], bv[4];
    
    // Bernstein-Polynome fuer U-Achse
    bu[0] = std::pow(1.0f - u, 3);
    bu[1] = 3.0f * u * std::pow(1.0f - u, 2);
    bu[2] = 3.0f * u * u * (1.0f - u);
    bu[3] = std::pow(u, 3);
    
    // Bernstein-Polynome fuer V-Achse
    bv[0] = std::pow(1.0f - v, 3);
    bv[1] = 3.0f * v * std::pow(1.0f - v, 2);
    bv[2] = 3.0f * v * v * (1.0f - v);
    bv[3] = std::pow(v, 3);
    
    // Punkte gewichten
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            p[0] += bu[i] * bv[j] * ctrlPoints[i][j][0];
            p[1] += bu[i] * bv[j] * ctrlPoints[i][j][1];
            p[2] += bu[i] * bv[j] * ctrlPoints[i][j][2];
        }
    }
    return p;
}

void drawBezierSurface() {
    // Wir zeichnen das Gitter aus den berechneten Punkten
    for (int i = 0; i < divs; i++) {
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= divs; j++) {
            float u1 = (float)i / divs;
            float u2 = (float)(i + 1) / divs;
            float v  = (float)j / divs;
            
            std::vector<float> p1 =bezierCalc(u1, v);
            std::vector<float> p2 =bezierCalc(u2, v);
            
            // Farbverlauf basierend auf der Hoehe (Z-Achse)
            glColor3f(0.0f, 0.5f + p1[2]*0.1f, 1.0f - p1[2]*0.1f);
            glVertex3f(p1[0], p1[1], p1[2]);
            
            glColor3f(0.0f, 0.5f + p2[2]*0.1f, 1.0f - p2[2]*0.1f);
            glVertex3f(p2[0], p2[1], p2[2]);
        }
        glEnd();
    }
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 28 - Bezier Surfaces", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef(rX, 1.0f, 0.0f, 0.0f); glRotatef(rY, 0.0f, 1.0f, 0.0f);

        // Zeichne die geschwungene Flaeche im Wireframe-Modus (Gitter-Modus)
        // Dadurch sieht man die mathematischen Linien perfekt!
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawBezierSurface();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Zuruecksetzen

        rX += 0.3f; rY += 0.5f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}