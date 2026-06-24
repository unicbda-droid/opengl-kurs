#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>

float rY = 0.0f;

GLfloat cubeVertices[] = {
    -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f
};

GLfloat cubeColors[] = {
    1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,
    0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,
    0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,
    1.0f,1.0f,0.0f,  1.0f,1.0f,0.0f,  1.0f,1.0f,0.0f,  1.0f,1.0f,0.0f,
    0.0f,1.0f,1.0f,  0.0f,1.0f,1.0f,  0.0f,1.0f,1.0f,  0.0f,1.0f,1.0f,
    1.0f,0.0f,1.0f,  1.0f,0.0f,1.0f,  1.0f,0.0f,1.0f,  1.0f,0.0f,1.0f
};

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 32 - Vertex Arrays", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w); 
    glEnable(GL_DEPTH_TEST);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, cubeVertices);
    glColorPointer(3, GL_FLOAT, 0, cubeColors);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -4.0f);
        glRotatef(rY, 1.0f, 1.0f, 0.0f);

        glDrawArrays(GL_QUADS, 0, 24);

        rY += 0.5f;
        glfwSwapBuffers(w); glfwPollEvents();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glfwTerminate(); return 0;
}
