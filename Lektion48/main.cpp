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
            GLubyte r = (GLubyte)((std::sin(x * 0.5f) * 0.5f + 0.5f) * 120 + 100);
            GLubyte g = (GLubyte)((std::cos(y * 0.5f) * 0.5f + 0.5f) * 150 + 80);
            GLubyte b = (GLubyte)((x ^ y) % 255);
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

void createIndexedTerrain() {
    std::vector<Vertex3D> vertices;
    std::vector<GLuint> indices;

    // 1. SCHRITT: Jeder Punkt wird exakt EINMAL generiert (Keine Duplikate!)
    for(int z = 0; z < GRID_SIZE; z++) {
        for(int x = 0; x < GRID_SIZE; x++) {
            float xPos = (float)x - (GRID_SIZE / 2.0f);
            float zPos = (float)z - (GRID_SIZE / 2.0f);
            
            float yVal = std::sin(xPos * 0.15f) * std::cos(zPos * 0.15f) * 4.0f;
            yVal += std::sin(xPos * 0.5f) * 0.8f; 

            float dx = 0.15f * std::cos(xPos * 0.15f) * std::cos(zPos * 0.15f) + 0.5f * std::cos(xPos * 0.5f);
            float dz = -0.15f * std::sin(xPos * 0.15f) * std::sin(zPos * 0.15f);
            float nx = -dx, ny = 1.0f, nz = -dz;
            float length = std::sqrt(nx*nx + ny*ny + nz*nz);
            nx /= length; ny /= length; nz /= length;

            float u = xPos * 0.2f; float v = zPos * 0.2f;
            vertices.push_back({xPos, yVal, zPos, nx, ny, nz, u, v});
        }
    }

    // 2. SCHRITT: Die Index-Tabelle bauen, um die Punkte zu Dreiecken zu verbinden
    for(int z = 0; z < GRID_SIZE - 1; z++) {
        for(int x = 0; x < GRID_SIZE - 1; x++) {
            // Die vier Ecken eines Grid-Quads bestimmen
            GLuint topLeft     = z * GRID_SIZE + x;
            GLuint topRight    = topLeft + 1;
            GLuint bottomLeft  = (z + 1) * GRID_SIZE + x;
            GLuint bottomRight = bottomLeft + 1;

            // Erstes Dreieck des Quads
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Zweites Dreieck des Quads
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    totalIndices = indices.size();

    // Vertices in den Standard-VRAM-Buffer (GL_ARRAY_BUFFER) laden
    glGenBuffers(1, &vboID);
    glBindBuffer(0x8892, vboID); 
    glBufferData(0x8892, vertices.size() * sizeof(Vertex3D), vertices.data(), 0x88E4);

    // Indizes in den Element-Buffer (GL_ELEMENT_ARRAY_BUFFER = 0x8893) jagen!
    glGenBuffers(1, &iboID);
    glBindBuffer(0x8893, iboID); 
    glBufferData(0x8893, indices.size() * sizeof(GLuint), indices.data(), 0x88E4);

    // Entkoppeln
    glBindBuffer(0x8892, 0);
    glBindBuffer(0x8893, 0);
}

void moveCamera(float speed, float angleOffset) {
    float rad = (yaw + angleOffset) * M_PI / 180.0f;
    camX -= std::sin(rad) * speed;
    camZ -= std::cos(rad) * speed;
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 48 - Ultra-Optimiertes Index-VBO (EBO)", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    createProceduralTexture();
    createIndexedTerrain();

    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_TEXTURE_2D);

    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.25f, 1.0f }; 
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 0.95f, 1.0f }; 
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 200.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;

        if(glfwGetKey(w, GLFW_KEY_UP)==GLFW_PRESS)    pitch += 1.5f;
        if(glfwGetKey(w, GLFW_KEY_DOWN)==GLFW_PRESS)  pitch -= 1.5f;
        if(glfwGetKey(w, GLFW_KEY_LEFT)==GLFW_PRESS)  yaw += 1.5f;
        if(glfwGetKey(w, GLFW_KEY_RIGHT)==GLFW_PRESS) yaw -= 1.5f;
        if(glfwGetKey(w, GLFW_KEY_W)==GLFW_PRESS) moveCamera(0.3f, 0.0f);
        if(glfwGetKey(w, GLFW_KEY_S)==GLFW_PRESS) moveCamera(0.3f, 180.0f);

        glClearColor(0.02f, 0.02f, 0.04f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 

        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glTranslatef(-camX, -camY, -camZ);

        GLfloat lightPosition[] = { 30.0f, 40.0f, 20.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

        glColor3f(1.0f, 1.0f, 1.0f);

        // BEIDE BUFFER AKTIVIEREN
        glBindBuffer(0x8892, vboID); // Vertex Buffer
        glBindBuffer(0x8893, iboID); // Index/Element Buffer
        glBindTexture(GL_TEXTURE_2D, textureID);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY); 
        glEnableClientState(GL_TEXTURE_COORD_ARRAY); 

        glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), (void*)0);
        glNormalPointer(GL_FLOAT, sizeof(Vertex3D), (void*)(sizeof(float) * 3)); 
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3D), (void*)(sizeof(float) * 6));

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        // JETZT NEU: Kein Loop mehr, kein Array-Strip! Wir zeichnen ALLES mit einem einzigen Befehl via Indizes!
        // 0x1405 entspricht GL_UNSIGNED_INT
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
