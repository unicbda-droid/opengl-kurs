#define _USE_MATH_DEFINES
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <iostream>
#include <vector>

float camX = 0.0f, camY = 3.0f, camZ = 15.0f;
float pitch = 15.0f, yaw = 0.0f;
float terrainTime = 0.0f;
const int GRID_SIZE = 30;

// Die Position unserer "Sonne" im 3D-Raum (genau im Zentrum ueber dem Gitter)
float sunX = 0.0f, sunY = 4.0f, sunZ = 0.0f;

struct Vertex3D { float x, y, z; float r, g, b; };

void moveCamera(float speed, float angleOffset) {
    float rad = (yaw + angleOffset) * M_PI / 180.0f;
    camX -= std::sin(rad) * speed;
    camZ -= std::cos(rad) * speed;
}

// Hilfsfunktion zum Zeichnen eines sauberen, ungeglaetteten 2D-Kreises (Halo)
void drawHalo(float x, float y, float radius, float r, float g, float b, float alpha) {
    glColor4f(r, g, b, alpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // Mittelpunkt
    for(int i = 0; i <= 20; i++) {
        float angle = i * 2.0f * M_PI / 20.0f;
        glVertex2f(x + std::cos(angle) * radius, y + std::sin(angle) * radius);
    }
    glEnd();
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 44 - Geometrische Lens Flares", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w); 
    glEnable(GL_DEPTH_TEST);

    // Additives Blending fuer das pure Leuchten der Flares
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 150.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;

        // Kamera-Steuerung (W,A,S,D + Pfeiltasten)
        if(glfwGetKey(w, GLFW_KEY_UP)==GLFW_PRESS)    pitch += 1.5f;
        if(glfwGetKey(w, GLFW_KEY_DOWN)==GLFW_PRESS)  pitch -= 1.5f;
        if(glfwGetKey(w, GLFW_KEY_LEFT)==GLFW_PRESS)  yaw += 1.5f;
        if(glfwGetKey(w, GLFW_KEY_RIGHT)==GLFW_PRESS) yaw -= 1.5f;
        if(glfwGetKey(w, GLFW_KEY_W)==GLFW_PRESS) moveCamera(0.15f, 0.0f);
        if(glfwGetKey(w, GLFW_KEY_S)==GLFW_PRESS) moveCamera(0.15f, 180.0f);

        glClearColor(0.01f, 0.01f, 0.02f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 

        // 1. WELT-TRANSFORMATION (Kamera)
        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glTranslatef(-camX, -camY, -camZ);

        // 2. GELÄNDE ZEICHNEN
        std::vector<Vertex3D> terrainGrid;
        for(int z = 0; z < GRID_SIZE - 1; z++) {
            for(int x = 0; x < GRID_SIZE; x++) {
                float xPos = (float)x - (GRID_SIZE / 2.0f);
                float zPos1 = (float)z - (GRID_SIZE / 2.0f);
                float zPos2 = (float)(z + 1) - (GRID_SIZE / 2.0f);
                float yVal1 = std::sin(std::sqrt(xPos*xPos + zPos1*zPos1) * 0.3f - terrainTime) * 1.2f;
                float yVal2 = std::sin(std::sqrt(xPos*xPos + zPos2*zPos2) * 0.3f - terrainTime) * 1.2f;
                float cf1 = (yVal1 + 1.2f) / 2.4f;
                float cf2 = (yVal2 + 1.2f) / 2.4f;
                terrainGrid.push_back({xPos, yVal1, zPos1, 0.0f, cf1, 1.0f - cf1});
                terrainGrid.push_back({xPos, yVal2, zPos2, 0.0f, cf2, 1.0f - cf2});
            }
        }
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), &terrainGrid[0].x);
        glColorPointer(3, GL_FLOAT, sizeof(Vertex3D), &terrainGrid[0].r);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for(int i = 0; i < GRID_SIZE - 1; i++) glDrawArrays(GL_TRIANGLE_STRIP, i * (GRID_SIZE * 2), GRID_SIZE * 2);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        // 3. DIE SONNE ZEICHNEN (Als fetter, knackiger Punkt im Raum)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_QUADS);
        glColor4f(1.0f, 1.0f, 0.9f, 1.0f);
        glVertex3f(sunX-0.3f, sunY-0.3f, sunZ); glVertex3f(sunX+0.3f, sunY-0.3f, sunZ);
        glVertex3f(sunX+0.3f, sunY+0.3f, sunZ); glVertex3f(sunX-0.3f, sunY+0.3f, sunZ);
        glEnd();

        // 4. LENS FLARE (Berechnung im 2D Screen Space nach dem 3D-Rendering)
        // Wir tricksen hier mathematisch, um die Sonnenposition grob auf dem Screen zu simulieren
        float radYaw = yaw * M_PI / 180.0f;
        float radPitch = pitch * M_PI / 180.0f;
        
        // Einfache Projektion der Drehung auf den Bildschirm (-1.0 bis 1.0)
        float screenSunX = -std::sin(radYaw); 
        float screenSunY = std::sin(radPitch) - 0.2f;

        // Wenn die Sonne halbwegs im Sichtfeld ist, zeichnen wir die Flares
        if (std::cos(radYaw) > 0.3f) {
            // Matrix wechseln auf flaches 2D-Overlay (Orthographisch)
            glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
            glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
            glDisable(GL_DEPTH_TEST);

            // Mittelpunkt des Bildschirms ist (0,0)
            float centerX = 0.0f;
            float centerY = 0.0f;

            // Vektor von der Sonne zur Mitte
            float vecX = centerX - screenSunX;
            float vecY = centerY - screenSunY;

            // Zeichne verschiedene Halos entlang des Vektors
            drawHalo(screenSunX + vecX * 0.3f, screenSunY + vecY * 0.3f, 0.08f, 0.0f, 0.8f, 1.0f, 0.3f); // Cyan Halo
            drawHalo(screenSunX + vecX * 0.6f, screenSunY + vecY * 0.6f, 0.04f, 1.0f, 0.5f, 0.0f, 0.4f); // Orange Flare
            drawHalo(screenSunX + vecX * 1.2f, screenSunY + vecY * 1.2f, 0.15f, 0.0f, 0.4f, 1.0f, 0.2f); // Grosser Ring
            drawHalo(screenSunX + vecX * 1.5f, screenSunY + vecY * 1.5f, 0.02f, 1.0f, 0.0f, 0.5f, 0.5f); // Kleiner Punkt

            glEnable(GL_DEPTH_TEST);
            glMatrixMode(GL_PROJECTION); glPopMatrix();
            glMatrixMode(GL_MODELVIEW); glPopMatrix();
        }

        terrainTime += 0.04f;
        glfwSwapBuffers(w); glfwPollEvents();
    }

    glfwTerminate(); return 0;
}
