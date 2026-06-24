#define _USE_MATH_DEFINES
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <iostream>
#include <vector>

// Kamera-Variablen (Position im Raum)
float camX = 0.0f, camY = 2.0f, camZ = 15.0f;
// Kamera-Rotation (Blickwinkel)
float pitch = 15.0f; // Leicht nach unten schauen beim Start
float yaw = 0.0f;

float terrainTime = 0.0f;
const int GRID_SIZE = 40; // Etwas groesser fuer mehr Flugraum

struct Vertex3D {
    float x, y, z;
    float r, g, b;
};

// Hilfsfunktion fuer die Bewegung in Blickrichtung
void moveCamera(float speed, float angleOffset) {
    float rad = (yaw + angleOffset) * M_PI / 180.0f;
    camX -= std::sin(rad) * speed;
    camZ -= std::cos(rad) * speed;
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 43 - Freie 3D Ego-Kamera", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w); 
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 150.0f);
    glMatrixMode(GL_MODELVIEW);

    std::cout << "STEUERUNG:" << std::endl;
    std::cout << "  W / S : Vorwaerts / Rueckwaerts fliegen" << std::endl;
    std::cout << "  A / D : Seitwaerts strafen" << std::endl;
    std::cout << "  PFEILTASTEN : Umschauen (Hoch, Runter, Links, Rechts)" << std::endl;

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;

        // ---- TASTATUR-ABFRAGE FÜR DIE KAMERA ----
        float moveSpeed = 0.15f;
        float rotateSpeed = 1.5f;

        // Umschauen (Pfeiltasten)
        if(glfwGetKey(w, GLFW_KEY_UP)==GLFW_PRESS)    pitch += rotateSpeed;
        if(glfwGetKey(w, GLFW_KEY_DOWN)==GLFW_PRESS)  pitch -= rotateSpeed;
        if(glfwGetKey(w, GLFW_KEY_LEFT)==GLFW_PRESS)  yaw += rotateSpeed;
        if(glfwGetKey(w, GLFW_KEY_RIGHT)==GLFW_PRESS) yaw -= rotateSpeed;

        // Grenzen fuer das Nicken (nicht ueberkopf fliegen)
        if(pitch > 89.0f) pitch = 89.0f;
        if(pitch < -89.0f) pitch = -89.0f;

        // Bewegen (W, A, S, D)
        if(glfwGetKey(w, GLFW_KEY_W)==GLFW_PRESS) moveCamera(moveSpeed, 0.0f);    // Vorwaerts
        if(glfwGetKey(w, GLFW_KEY_S)==GLFW_PRESS) moveCamera(moveSpeed, 180.0f);  // Rueckwaerts
        if(glfwGetKey(w, GLFW_KEY_A)==GLFW_PRESS) moveCamera(moveSpeed, 90.0f);   // Links strafen
        if(glfwGetKey(w, GLFW_KEY_D)==GLFW_PRESS) moveCamera(moveSpeed, 270.0f);  // Rechts strafen

        glClearColor(0.01f, 0.01f, 0.02f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 

        // ---- DIE KAMERA-MATRIX ANWENDEN ----
        // Zuerst die Rotation umkehren, dann die Position verschieben
        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glTranslatef(-camX, -camY, -camZ);

        // ---- GEOMETRIE ZEICHNEN (Glaenzendes Wellennetz) ----
        std::vector<Vertex3D> terrainGrid;
        for(int z = 0; z < GRID_SIZE - 1; z++) {
            for(int x = 0; x < GRID_SIZE; x++) {
                float xPos1 = (float)x - (GRID_SIZE / 2.0f);
                float zPos1 = (float)z - (GRID_SIZE / 2.0f);
                float zPos2 = (float)(z + 1) - (GRID_SIZE / 2.0f);

                float yVal1 = std::sin(std::sqrt(xPos1*xPos1 + zPos1*zPos1) * 0.3f - terrainTime) * 1.5f;
                float yVal2 = std::sin(std::sqrt(xPos1*xPos1 + zPos2*zPos2) * 0.3f - terrainTime) * 1.5f;

                float colorFactor1 = (yVal1 + 1.5f) / 3.0f;
                float colorFactor2 = (yVal2 + 1.5f) / 3.0f;

                terrainGrid.push_back({xPos1, yVal1, zPos1, 0.0f, colorFactor1, 1.0f - colorFactor1});
                terrainGrid.push_back({xPos1, yVal2, zPos2, 0.0f, colorFactor2, 1.0f - colorFactor2});
            }
        }

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), &terrainGrid[0].x);
        glColorPointer(3, GL_FLOAT, sizeof(Vertex3D), &terrainGrid[0].r);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for(int i = 0; i < GRID_SIZE - 1; i++) {
            glDrawArrays(GL_TRIANGLE_STRIP, i * (GRID_SIZE * 2), GRID_SIZE * 2);
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        terrainTime += 0.05f;
        
        glfwSwapBuffers(w); glfwPollEvents();
    }

    glfwTerminate(); return 0;
}
