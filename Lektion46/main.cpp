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
};

GLuint vboID = 0;
int totalVertices = 0;

void createIlluminatedTerrain() {
    std::vector<Vertex3D> vertices;

    for(int z = 0; z < GRID_SIZE - 1; z++) {
        for(int x = 0; x < GRID_SIZE; x++) {
            for(int row = 0; row < 2; row++) {
                int currentZ = z + row;
                float xPos = (float)x - (GRID_SIZE / 2.0f);
                float zPos = (float)currentZ - (GRID_SIZE / 2.0f);
                
                float yVal = std::sin(xPos * 0.15f) * std::cos(zPos * 0.15f) * 4.0f;
                yVal += std::sin(xPos * 0.5f) * 0.8f; 

                float dx = 0.15f * std::cos(xPos * 0.15f) * std::cos(zPos * 0.15f) + 0.5f * std::cos(xPos * 0.5f);
                float dz = -0.15f * std::sin(xPos * 0.15f) * std::sin(zPos * 0.15f);
                
                float nx = -dx;
                float ny = 1.0f;
                float nz = -dz;
                float length = std::sqrt(nx*nx + ny*ny + nz*nz);
                nx /= length; ny /= length; nz /= length;

                vertices.push_back({xPos, yVal, zPos, nx, ny, nz});
            }
        }
    }

    totalVertices = vertices.size();

    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex3D), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void moveCamera(float speed, float angleOffset) {
    float rad = (yaw + angleOffset) * M_PI / 180.0f;
    camX -= std::sin(rad) * speed;
    camZ -= std::cos(rad) * speed;
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 46 - Echtes Sonnenlicht & Schatten-Gefaelle", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    createIlluminatedTerrain();

    glEnable(GL_LIGHTING); 
    glEnable(GL_LIGHT0);   
    glEnable(GL_COLOR_MATERIAL); 

    GLfloat lightAmbient[]  = { 0.1f, 0.12f, 0.15f, 1.0f }; 
    GLfloat lightDiffuse[]  = { 0.9f, 0.85f, 0.75f, 1.0f }; 
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

        glClearColor(0.05f, 0.08f, 0.12f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 

        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glTranslatef(-camX, -camY, -camZ);

        GLfloat lightPosition[] = { 20.0f, 30.0f, 20.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

        glColor3f(0.1f, 0.7f, 0.4f);

        glBindBuffer(GL_ARRAY_BUFFER, vboID);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY); 

        glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), (void*)0);
        glNormalPointer(GL_FLOAT, sizeof(Vertex3D), (void*)(sizeof(float) * 3)); 

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        for(int i = 0; i < GRID_SIZE - 1; i++) {
            glDrawArrays(GL_TRIANGLE_STRIP, i * (GRID_SIZE * 2), GRID_SIZE * 2);
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glfwSwapBuffers(w); glfwPollEvents();
    }

    glfwTerminate(); return 0;
}
