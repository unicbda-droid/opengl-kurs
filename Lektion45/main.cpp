#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <vector>

float camX = 0.0f, camY = 15.0f, camZ = 40.0f;
float pitch = 25.0f, yaw = 0.0f;
const int GRID_SIZE = 100;

struct Vertex3D {
    float x, y, z;
    float r, g, b;
};

GLuint vboID = 0;
int totalVertices = 0;

void createMountainTerrain() {
    std::vector<Vertex3D> vertices;

    // Ein riesiges, raues Gebirge berechnen
    for(int z = 0; z < GRID_SIZE - 1; z++) {
        for(int x = 0; x < GRID_SIZE; x++) {
            for(int row = 0; row < 2; row++) {
                int currentZ = z + row;
                float xPos = (float)x - (GRID_SIZE / 2.0f);
                float zPos = (float)currentZ - (GRID_SIZE / 2.0f);
                
                // Komplexe, ueberlagerte Sinus-Funktionen fuer echt wirkende Berge
                float yVal = std::sin(xPos * 0.15f) * std::cos(zPos * 0.15f) * 4.0f;
                yVal += std::sin(xPos * 0.5f) * 1.0f; // Detail-Rauschen

                // Schnee auf den Spitzen (Weiss), Gras im Tal (Gruen)
                float r = 0.0f, g = 0.3f, b = 0.5f;
                if(yVal > 2.0f) { r = 0.8f; g = 0.8f; b = 0.9f; } // Schneekappen
                else if(yVal < -1.0f) { r = 0.0f; g = 0.1f; b = 0.2f; } // Tiefe Taeler

                vertices.push_back({xPos, yVal, zPos, r, g, b});
            }
        }
    }

    totalVertices = vertices.size();

    // HIER PASSIERT DIE MAGIE: Daten zum VRAM schicken
    glGenBuffers(1, &vboID);
    glBindBuffer(0x8892, vboID); // 0x8892 entspricht GL_ARRAY_BUFFER
    glBufferData(0x8892, vertices.size() * sizeof(Vertex3D), vertices.data(), 0x88E4); // 0x88E4 entspricht GL_STATIC_DRAW
    
    // Buffer entkoppeln, Daten liegen jetzt sicher auf der GPU
    glBindBuffer(0x8892, 0);
}

void moveCamera(float speed, float angleOffset) {
    float rad = (yaw + angleOffset) * M_PI / 180.0f;
    camX -= std::sin(rad) * speed;
    camZ -= std::cos(rad) * speed;
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 45 - High-Speed Vertex Buffer Objects (VBO)", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    createMountainTerrain();

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 200.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;

        // Steuerung (Fliege frei durch die VBO-Berglandschaft)
        if(glfwGetKey(w, GLFW_KEY_UP)==GLFW_PRESS)    pitch += 1.5f;
        if(glfwGetKey(w, GLFW_KEY_DOWN)==GLFW_PRESS)  pitch -= 1.5f;
        if(glfwGetKey(w, GLFW_KEY_LEFT)==GLFW_PRESS)  yaw += 1.5f;
        if(glfwGetKey(w, GLFW_KEY_RIGHT)==GLFW_PRESS) yaw -= 1.5f;
        if(glfwGetKey(w, GLFW_KEY_W)==GLFW_PRESS) moveCamera(0.3f, 0.0f);
        if(glfwGetKey(w, GLFW_KEY_S)==GLFW_PRESS) moveCamera(0.3f, 180.0f);

        glClearColor(0.01f, 0.02f, 0.04f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 

        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glTranslatef(-camX, -camY, -camZ);

        // RENDERN DIREKT AUS DEM VRAM
        glBindBuffer(0x8892, vboID); // Buffer aktivieren

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        // Die Pointer zeigen jetzt nicht mehr auf ein Array im RAM, sondern auf Offsets im VRAM!
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), (void*)0);
        glColorPointer(3, GL_FLOAT, sizeof(Vertex3D), (void*)(sizeof(float) * 3));

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        // Das komplette Gebirge mit einem einzigen Befehl zeichnen
        for(int i = 0; i < GRID_SIZE - 1; i++) {
            glDrawArrays(GL_TRIANGLE_STRIP, i * (GRID_SIZE * 2), GRID_SIZE * 2);
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        
        glBindBuffer(0x8892, 0); // Buffer wieder freigeben

        glfwSwapBuffers(w); glfwPollEvents();
    }

    if(vboID != 0) glDeleteBuffers(1, &vboID);
    glfwTerminate(); return 0;
}
