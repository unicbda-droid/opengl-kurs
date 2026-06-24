#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <vector>

const int GRID_SIZE = 60;

struct FinalVertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

GLuint vboID = 0;
GLuint iboID = 0;
GLuint textureID = 0;
int totalIndices = 0;

void generateProceduralTexture() {
    const int texWidth = 256;
    const int texHeight = 256;
    std::vector<unsigned char> pixelData(texWidth * texHeight * 3);

    for (int y = 0; y < texHeight; y++) {
        for (int x = 0; x < texWidth; x++) {
            int idx = (y * texWidth + x) * 3;
            bool checker = ((x / 16) % 2 == 0) ^ ((y / 16) % 2 == 0);
            
            if (checker) {
                pixelData[idx]     = 240; // R
                pixelData[idx + 1] = 190; // G
                pixelData[idx + 2] = 40;  // B (Gelb-Orange aus Lektion 47/48)
            } else {
                pixelData[idx]     = 200; // R
                pixelData[idx + 1] = 40;  // G
                pixelData[idx + 2] = 40;  // B (Rot-Nuance)
            }
        }
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData.data());
}

void updateGeometry(std::vector<FinalVertex>& vertices, float time) {
    vertices.clear();
    for(int z = 0; z < GRID_SIZE; z++) {
        for(int x = 0; x < GRID_SIZE; x++) {
            float xPos = (float)x - (GRID_SIZE / 2.0f);
            float zPos = (float)z - (GRID_SIZE / 2.0f);
            
            // Die Welle
            float yVal = std::sin(xPos * 0.2f + time * 2.0f) * std::cos(zPos * 0.2f + time * 1.5f) * 3.5f;

            // Ableitung für die echten Licht-Normalen (Lektion 46 Kreuzprodukt-Ersatz)
            float dx = 0.2f * std::cos(xPos * 0.2f + time * 2.0f) * std::cos(zPos * 0.2f + time * 1.5f);
            float dz = -0.2f * std::sin(xPos * 0.2f + time * 2.0f) * std::sin(zPos * 0.2f + time * 1.5f);
            float nx = -dx, ny = 1.0f, nz = -dz;
            float length = std::sqrt(nx*nx + ny*ny + nz*nz);
            nx /= length; ny /= length; nz /= length;

            // UVs kacheln
            float u = (float)x * 0.15f;
            float v = (float)z * 0.15f;
            
            vertices.push_back({xPos, yVal, zPos, nx, ny, nz, u, v});
        }
    }
}

void initBuffers() {
    std::vector<FinalVertex> dummy(GRID_SIZE * GRID_SIZE);
    std::vector<GLuint> indices;
    for(int z = 0; z < GRID_SIZE - 1; z++) {
        for(int x = 0; x < GRID_SIZE - 1; x++) {
            GLuint topLeft     = z * GRID_SIZE + x;
            GLuint topRight    = topLeft + 1;
            GLuint bottomLeft  = (z + 1) * GRID_SIZE + x;
            GLuint bottomRight = bottomLeft + 1;
            
            indices.push_back(topLeft); indices.push_back(bottomLeft); indices.push_back(topRight);
            indices.push_back(topRight); indices.push_back(bottomLeft); indices.push_back(bottomRight);
        }
    }
    totalIndices = indices.size();
    
    glGenBuffers(1, &vboID); glBindBuffer(0x8892, vboID);
    glBufferData(0x8892, dummy.size() * sizeof(FinalVertex), nullptr, 0x88E8); 
    glGenBuffers(1, &iboID); glBindBuffer(0x8893, iboID); 
    glBufferData(0x8893, indices.size() * sizeof(GLuint), indices.data(), 0x88E4);
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 50 - Das fette Finale", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // LICHT UND TEXTUR GLEICHZEITIG AKTIVIEREN!
    glEnable(GL_LIGHTING); 
    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);

    // Licht-Farben einstellen
    GLfloat lightAmbient[] = { 0.25f, 0.25f, 0.3f, 1.0f }; 
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 0.95f, 1.0f }; // Leicht warmes Sonnenlicht
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    initBuffers();
    generateProceduralTexture();

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); 
    glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 500.0f);
    glMatrixMode(GL_MODELVIEW);

    std::vector<FinalVertex> vertices;

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;

        float time = glfwGetTime();
        updateGeometry(vertices, time);

        glBindBuffer(0x8892, vboID);
        glBufferSubData(0x8892, 0, vertices.size() * sizeof(FinalVertex), vertices.data());

        // Hintergrund dunkelblau-grau
        glClearColor(0.05f, 0.06f, 0.09f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 

        // Die bewährte Kameraposition
        glTranslatef(0.0f, -10.0f, -45.0f);
        glRotatef(35.0f, 1.0f, 0.0f, 0.0f);

        // Position der Sonne am Himmel einfrieren
        GLfloat lightPosition[] = { 20.0f, 60.0f, 10.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

        glColor3f(1.0f, 1.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBindBuffer(0x8893, iboID); 

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, sizeof(FinalVertex), (void*)0);
        glNormalPointer(GL_FLOAT, sizeof(FinalVertex), (void*)(sizeof(float) * 3));
        glTexCoordPointer(2, GL_FLOAT, sizeof(FinalVertex), (void*)(sizeof(float) * 6));

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, totalIndices, 0x1405, (void*)0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glfwSwapBuffers(w); glfwPollEvents();
    }

    glfwTerminate(); return 0;
}
