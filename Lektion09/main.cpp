#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <vector>
#include <iostream>

// Kamera- und Bewegungsvariablen
float xpos = 0.0f;
float zpos = 0.0f;
float yrot = 0.0f;
float lookupdown = 0.0f;

const float piover180 = 0.0174532925f;
GLuint textureID;

struct Vertex {
    float x, y, z;
    float u, v;
};

std::vector<Vertex> worldSector;

void buildWorld() {
    // Wir bauen einen einfachen Raum aus Quads (Boden, Decke, 4 Wände)
    // Format: X, Y, Z, U, V
    Vertex v[] = {
        // Boden
        {-10.0f, -1.0f, -10.0f, 0.0f, 10.0f}, { 10.0f, -1.0f, -10.0f, 10.0f, 10.0f},
        { 10.0f, -1.0f,  10.0f, 10.0f,  0.0f}, {-10.0f, -1.0f,  10.0f, 0.0f,  0.0f},
        // Decke
        {-10.0f,  1.0f, -10.0f, 0.0f, 10.0f}, {-10.0f,  1.0f,  10.0f, 0.0f,  0.0f},
        { 10.0f,  1.0f,  10.0f, 10.0f,  0.0f}, { 10.0f,  1.0f, -10.0f, 10.0f, 10.0f},
        // Wand Nord
        {-10.0f,  1.0f, -10.0f, 0.0f,  1.0f}, { 10.0f,  1.0f, -10.0f,  4.0f,  1.0f},
        { 10.0f, -1.0f, -10.0f, 4.0f,  0.0f}, {-10.0f, -1.0f, -10.0f, 0.0f,  0.0f},
        // Wand Süd
        {-10.0f,  1.0f,  10.0f, 0.0f,  1.0f}, {-10.0f, -1.0f,  10.0f, 0.0f,  0.0f},
        { 10.0f, -1.0f,  10.0f, 4.0f,  0.0f}, { 10.0f,  1.0f,  10.0f, 4.0f,  1.0f},
        // Wand West
        {-10.0f,  1.0f,  10.0f, 4.0f,  1.0f}, {-10.0f,  1.0f, -10.0f, 0.0f,  1.0f},
        {-10.0f, -1.0f, -10.0f, 0.0f,  0.0f}, {-10.0f, -1.0f,  10.0f, 4.0f,  0.0f},
        // Wand Ost
        { 10.0f,  1.0f,  10.0f, 4.0f,  1.0f}, { 10.0f, -1.0f,  10.0f, 4.0f,  0.0f},
        { 10.0f, -1.0f, -10.0f, 0.0f,  0.0f}, { 10.0f,  1.0f, -10.0f, 0.0f,  1.0f}
    };
    worldSector.assign(v, v + sizeof(v) / sizeof(Vertex));
}

void createProceduralTexture() {
    const int width = 64;
    const int height = 64;
    std::vector<GLubyte> image(width * height * 3);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            GLubyte c = ((((i & 0x4) == 0) ^ ((j & 0x4) == 0))) * 255;
            int index = (i * width + j) * 3;
            image[index]     = c * 0.3f; 
            image[index + 1] = c * 0.6f; 
            image[index + 2] = c;        
        }
    }
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data());
}

void handleInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    float speed = 0.05f;
    
    // Drehung nach links/rechts
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yrot -= 1.5f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  yrot += 1.5f;

    // Vorwärts / Rückwärts (mit Trigonometrie in Blickrichtung laufen)
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        xpos -= std::sin(yrot * piover180) * speed;
        zpos -= std::cos(yrot * piover180) * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        xpos += std::sin(yrot * piover180) * speed;
        zpos += std::cos(yrot * piover180) * speed;
    }
}

int main() {
    if (!glfwInit()) return -1;
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "NeHe Lektion 10 - Deine erste 3D-Welt", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    createProceduralTexture();
    buildWorld();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLfloat aspect = 800.0f / 600.0f;
    GLfloat fFOV = 45.0f * 3.14159265f / 180.0f;
    GLfloat fH = std::tan(fFOV / 2.0f) * 0.1f;
    GLfloat fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);

    while (!glfwWindowShouldClose(window)) {
        handleInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // Die Kamera-Transformation (Welt invers bewegen)
        glRotatef(lookupdown, 1.0f, 0.0f, 0.0f);
        glRotatef(360.0f - yrot, 0.0f, 1.0f, 0.0f);
        glTranslatef(-xpos, 0.0f, -zpos);

        glBindTexture(GL_TEXTURE_2D, textureID);

        // Die Welt rendern
        glBegin(GL_QUADS);
        for (const auto& vertex : worldSector) {
            glTexCoord2f(vertex.u, vertex.v);
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteTextures(1, &textureID);
    glfwTerminate();
    return 0;
}