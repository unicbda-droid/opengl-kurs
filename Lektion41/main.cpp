#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <iostream>
#include <vector>

float rY = 0.0f;
float waveTime = 0.0f;

// Struktur fuer einen 3D-Punkt
struct Vertex3D {
    float x, y, z;
};

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 41 - Volumetrische Dynamische Geometrie", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w); 
    glEnable(GL_DEPTH_TEST);

    // Additives Blending fuer den "Glüheffekt" der Welle
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;

        glClearColor(0.02f, 0.02f, 0.04f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 
        glTranslatef(0.0f, 0.0f, -7.0f);
        glRotatef(rY, 0.0f, 1.0f, 0.3f);

        // DYNAMISCHE GEOMETRIE-BERECHNUNG (Die Ur-Mutter der Prozeduralen Generierung)
        std::vector<Vertex3D> dynamicStrip;
        int numPoints = 100;
        
        for(int i = 0; i < numPoints; i++) {
            float percent = (float)i / (float)numPoints;
            float x = -5.0f + percent * 10.0f;
            
            // Mathematische Sinus-Welle, die sich ueber die Zeit (waveTime) veraendert
            float y = std::sin(percent * 10.0f + waveTime) * 1.5f;
            float z = std::cos(percent * 5.0f + waveTime) * 1.0f;

            // Wir bauen ein "Band" (zwei versetzte Punkte pro Schritt)
            float thickness = 0.15f;
            dynamicStrip.push_back({x, y - thickness, z});
            dynamicStrip.push_back({x, y + thickness, z});
        }

        // Wir uebergeben die frisch im RAM berechneten Daten an die GPU
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, dynamicStrip.data());

        // Farbverlauf ueber das Band jagen (Neon-Violett zu Cyan)
        glBegin(GL_TRIANGLE_STRIP);
        for(int i = 0; i < dynamicStrip.size(); i++) {
            float t = (float)i / (float)dynamicStrip.size();
            glColor4f(t, 1.0f - t, 1.0f, 0.6f); // Halbe Transparenz fuer das Gluehen
            glArrayElement(i);
        }
        glEnd();

        glDisableClientState(GL_VERTEX_ARRAY);

        rY += 0.3f;
        waveTime += 0.05f; // Geschwindigkeit der Wellen-Animation
        
        glfwSwapBuffers(w); glfwPollEvents();
    }

    glfwTerminate(); return 0;
}
