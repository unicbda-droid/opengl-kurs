#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <iostream>
#include <vector>

float rY = 0.0f;
float terrainTime = 0.0f;

const int GRID_SIZE = 32; // 32x32 Punkte Raster

struct Vertex3D {
    float x, y, z;
    float r, g, b; // Farbe direkt im Vertex speichern
};

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 42 - Dynamische Heightmap-Landschaft", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w); 
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;

        glClearColor(0.01f, 0.02f, 0.03f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 
        glTranslatef(0.0f, -1.0f, -25.0f); // Etwas weiter weg, um das Gelaende zu sehen
        glRotatef(25.0f, 1.0f, 0.0f, 0.0f);  // Leicht von oben herab schauen
        glRotatef(rY, 0.0f, 1.0f, 0.0f);     // Um die eigene Achse drehen

        std::vector<Vertex3D> terrainGrid;

        // DYNAMISCHE GENERIERUNG DES GELÄNDES
        for(int z = 0; z < GRID_SIZE - 1; z++) {
            for(int x = 0; x < GRID_SIZE; x++) {
                
                // Berechne X/Z Koordinaten zentriert um den Ursprung
                float xPos1 = (float)x - (GRID_SIZE / 2.0f);
                float zPos1 = (float)z - (GRID_SIZE / 2.0f);
                float zPos2 = (float)(z + 1) - (GRID_SIZE / 2.0f);

                // Die Hoehe (Y) wird dynamisch durch Sinus/Cosinus berechnet
                float yVal1 = std::sin(std::sqrt(xPos1*xPos1 + zPos1*zPos1) * 0.4f - terrainTime) * 1.2f;
                float yVal2 = std::sin(std::sqrt(xPos1*xPos1 + zPos2*zPos2) * 0.4f - terrainTime) * 1.2f;

                // Farbberechnung basierend auf der Hoehe (Täler dunkelblau, Spitzen hellgrün/cyan)
                float colorFactor1 = (yVal1 + 1.2f) / 2.4f;
                float colorFactor2 = (yVal2 + 1.2f) / 2.4f;

                // Vertex fuer Reihe Z
                terrainGrid.push_back({xPos1, yVal1, zPos1, 0.0f, colorFactor1, 1.0f - colorFactor1});
                // Vertex fuer Reihe Z+1 (Triangle Strip Logik)
                terrainGrid.push_back({xPos1, yVal2, zPos2, 0.0f, colorFactor2, 1.0f - colorFactor2});
            }
        }

        // Arrays aktivieren
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        // Wir nutzen den "Stride"-Parameter (sizeof(Vertex3D)), da Position und Farbe im selben Vektor liegen (Interleaved)
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), &terrainGrid[0].x);
        glColorPointer(3, GL_FLOAT, sizeof(Vertex3D), &terrainGrid[0].r);

        // Zeichne das komplette Gelaende als Drahtgitter (Wireframe) fuer maximale Sichtbarkeit der Kurven
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        // Da wir ein quadratisches Netz aus Triangle Strips gebaut haben, rendern wir Reihe fuer Reihe
        for(int i = 0; i < GRID_SIZE - 1; i++) {
            glDrawArrays(GL_TRIANGLE_STRIP, i * (GRID_SIZE * 2), GRID_SIZE * 2);
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        rY += 0.2f;
        terrainTime += 0.06f; // Geschwindigkeit der Gelaendewellen
        
        glfwSwapBuffers(w); glfwPollEvents();
    }

    glfwTerminate(); return 0;
}
