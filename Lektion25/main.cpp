#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <cmath>
#include <cstdlib>

struct Vertex { float x, y, z; };

const int VERTS_COUNT = 441;
std::vector<Vertex> morphSource(VERTS_COUNT);
std::vector<Vertex> morphTarget(VERTS_COUNT);
std::vector<Vertex> currentMesh(VERTS_COUNT);

float rX = 0.0f, rY = 0.0f;
float morphProgress = 1.0f;
float morphSpeed = 0.02f;

// Generiert vier mathematische Objekte in dieselbe Punktwolken-Groesse
void generateShape(int type, std::vector<Vertex>& dest) {
    int idx = 0;
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 21; j++) {
            float u = (float)i / 20.0f * 2.0f * 3.14159265f;
            float v = (float)j / 20.0f * 3.14159265f - 1.57079632f;
            
            if (type == 0) { // Kugel
                dest[idx].x = std::cos(v) * std::cos(u);
                dest[idx].y = std::sin(v);
                dest[idx].z = std::cos(v) * std::sin(u);
            } else if (type == 1) { // Torus (Donut)
                float r1 = 1.0f, r2 = 0.3f;
                dest[idx].x = (r1 + r2 * std::cos(v)) * std::cos(u);
                dest[idx].y = r2 * std::sin(v);
                dest[idx].z = (r1 + r2 * std::cos(v)) * std::sin(u);
            } else if (type == 2) { // Ebene (Welle)
                dest[idx].x = ((float)i / 10.0f) - 1.0f;
                dest[idx].y = std::sin(u) * 0.2f;
                dest[idx].z = ((float)j / 10.0f) - 1.0f;
            } else { // Pyramiden-Chaos
                dest[idx].x = std::cos(u) * (1.0f - (float)j/20.0f);
                dest[idx].y = ((float)j / 10.0f) - 1.0f;
                dest[idx].z = std::sin(u) * (1.0f - (float)j/20.0f);
            }
            idx++;
        }
    }
}

void startMorph(int targetType) {
    morphSource = currentMesh;
    generateShape(targetType, morphTarget);
    morphProgress = 0.0f;
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 25 - 3D Morphing", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);
    
    // Start-Form ist die Kugel
    generateShape(0, currentMesh);
    morphSource = currentMesh;
    morphTarget = currentMesh;

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        // Tastenabfrage fuer Morph-Ziele
        if(glfwGetKey(w, GLFW_KEY_1)==GLFW_PRESS && morphProgress >= 1.0f) startMorph(0);
        if(glfwGetKey(w, GLFW_KEY_2)==GLFW_PRESS && morphProgress >= 1.0f) startMorph(1);
        if(glfwGetKey(w, GLFW_KEY_3)==GLFW_PRESS && morphProgress >= 1.0f) startMorph(2);
        if(glfwGetKey(w, GLFW_KEY_4)==GLFW_PRESS && morphProgress >= 1.0f) startMorph(3);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -3.5f);
        glRotatef(rX, 1.0f, 0.0f, 0.0f); glRotatef(rY, 0.0f, 1.0f, 0.0f);

        // Morph-Interpolation berechnen
        if (morphProgress < 1.0f) {
            morphProgress += morphSpeed;
            if (morphProgress > 1.0f) morphProgress = 1.0f;
            
            for (int i = 0; i < VERTS_COUNT; i++) {
                currentMesh[i].x = morphSource[i].x + (morphTarget[i].x - morphSource[i].x) * morphProgress;
                currentMesh[i].y = morphSource[i].y + (morphTarget[i].y - morphSource[i].y) * morphProgress;
                currentMesh[i].z = morphSource[i].z + (morphTarget[i].z - morphSource[i].z) * morphProgress;
            }
        }

        // Punktwolke zeichnen
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < VERTS_COUNT; i++) {
            // Farbe aendert sich basierend auf der Position
            glColor3f(std::abs(currentMesh[i].x), std::abs(currentMesh[i].y), 1.0f - std::abs(currentMesh[i].z));
            glVertex3f(currentMesh[i].x, currentMesh[i].y, currentMesh[i].z);
        }
        glEnd();

        rX += 0.2f; rY += 0.4f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}