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
    float nx, ny, nz; 
    float u, v;       
};

GLuint vboID = 0;
GLuint iboID = 0;
GLuint textureID = 0;
int totalIndices = 0;

void createProceduralTexture() {
    const int texWidth = 64; const int texHeight = 64;
    GLubyte texData[texWidth * texHeight * 3];
    for(int y = 0; y < texHeight; y++) {
        for(int x = 0; x < texWidth; x++) {
            GLubyte r = (GLubyte)((std::sin(x * 0.3f) * 0.5f + 0.5f) * 50 + 20);
            GLubyte g = (GLubyte)((std::cos(y * 0.3f) * 0.5f + 0.5f) * 100 + 50);
            GLubyte b = (GLubyte)((x ^ y) % 100 + 155); // Mehr Blauanteil fuer Wasser-Optik
            int index = (y * texWidth + x) * 3;
            texData[index] = r; texData[index + 1] = g; texData[index + 2] = b;
        }
    }
    glGenTextures(1, &textureID); glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
}

// Berechnet das Grid basierend auf der aktuellen Zeit
void updateTerrainGeometry(std::vector<Vertex3D>& vertices, float time) {
    vertices.clear();
    for(int z = 0; z < GRID_SIZE; z++) {
        for(int x = 0; x < GRID_SIZE; x++) {
            float xPos = (float)x - (GRID_SIZE / 2.0f);
            float zPos = (float)z - (GRID_SIZE / 2.0f);
            
            // Dynamische Wellen-Gleichung mit Zeit-Faktor!
            float yVal = std::sin(xPos * 0.2f + time * 2.0f) * std::cos(zPos * 0.2f + time * 1.5f) * 3.0f;
            yVal += std::sin(xPos * 0.6f + time * 3.5f) * 0.4f; 

            // Dynamische Normalen-Berechnung (wichtig, damit das Licht auf den Wellen mitschwimmt!)
            float dx = 0.2f * std::cos(xPos * 0.2f + time * 2.0f) * std::cos(zPos * 0.2f + time * 1.5f) + 0.6f * std::cos(xPos * 0.6f + time * 3.5f);
            float dz = -0.2f * std::sin(xPos * 0.2f + time * 2.0f) * std::sin(zPos * 0.2f + time * 1.5f);
            float nx = -dx, ny = 1.0f, nz = -dz;
            float length = std::sqrt(nx*nx + ny*ny + nz*nz);
            nx /= length; ny /= length; nz /= length;

            float u = xPos * 0.2f; float v = zPos * 0.2f;
            vertices.push_back({xPos, yVal, zPos, nx, ny, nz, u, v});
        }
    }
}

void initIndexedTerrain() {
    std::vector<Vertex3D> dummyVertices(GRID_SIZE * GRID_SIZE);
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

    glGenBuffers(1, &vboID);
    glBindBuffer(0x8892, vboID);
    // HIER KORRIGIERT: 0x88E8 entspricht GL_DYNAMIC_DRAW!
    glBufferData(0x8892, dummyVertices.size() * sizeof(Vertex3D), nullptr, 0x88E8); 

    glGenBuffers(1, &iboID);
    glBindBuffer(0x8893, iboID); 
    glBufferData(0x8893, indices.size() * sizeof(GLuint), indices.data(), 0x88E4); // Indizes bleiben statisch (0x88E4 = GL_STATIC_DRAW)

    glBindBuffer(0x8892, 0); glBindBuffer(0x8893, 0);
}

void moveCamera(float speed, float angleOffset) {
    float rad = (yaw + angleOffset) * M_PI / 180.0f;
    camX -= std::sin(rad) * speed;
    camZ -= std::cos(rad) * speed;
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 49 - Dynamisches VBO mit Wellen-Animation", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    createProceduralTexture();
    initIndexedTerrain();

    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_TEXTURE_2D);

    GLfloat lightAmbient[] = { 0.15f, 0.15f, 0.25f, 1.0f }; 
    GLfloat lightDiffuse[] = { 0.8f, 0.85f, 1.0f, 1.0f }; 
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 200.0f);
    glMatrixMode(GL_MODELVIEW);

    std::vector<Vertex3D> dynamicVertices;

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;

        if(glfwGetKey(w, GLFW_KEY_UP)==GLFW_PRESS)    pitch += 1.5f;
        if(glfwGetKey(w, GLFW_KEY_DOWN)==GLFW_PRESS)  pitch -= 1.5f;
        if(glfwGetKey(w, GLFW_KEY_LEFT)==GLFW_PRESS)  yaw += 1.5f;
        if(glfwGetKey(w, GLFW_KEY_RIGHT)==GLFW_PRESS) yaw -= 1.5f;
        if(glfwGetKey(w, GLFW_KEY_W)==GLFW_PRESS) moveCamera(0.3f, 0.0f);
        if(glfwGetKey(w, GLFW_KEY_S)==GLFW_PRESS) moveCamera(0.3f, 180.0f);

        float time = glfwGetTime();

        // 1. Neue Geometrie auf der CPU berechnen
        updateTerrainGeometry(dynamicVertices, time);

        // 2. Bestehenden VBO-Speicher im VRAM blitzschnell ueberschreiben!
        glBindBuffer(0x8892, vboID);
        glBufferSubData(0x8892, 0, dynamicVertices.size() * sizeof(Vertex3D), dynamicVertices.data());

        glClearColor(0.01f, 0.02f, 0.04f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 

        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glTranslatef(-camX, -camY, -camZ);

        GLfloat lightPosition[] = { 0.0f, 50.0f, 10.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

        glBindBuffer(0x8893, iboID); 
        glBindTexture(GL_TEXTURE_2D, textureID);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY); 
        glEnableClientState(GL_TEXTURE_COORD_ARRAY); 

        glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), (void*)0);
        glNormalPointer(GL_FLOAT, sizeof(Vertex3D), (void*)(sizeof(float) * 3)); 
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3D), (void*)(sizeof(float) * 6));

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, totalIndices, 0x1405, (void*)0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindBuffer(0x8892, 0);
        glBindBuffer(0x8893, 0);

        glfwSwapBuffers(w); glfwPollEvents();
    }

    glfwTerminate(); return 0;
}
