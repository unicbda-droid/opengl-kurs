#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)width / (float)height;
    float fH = std::tan(45.0f / 360.0f * 3.14159f) * 0.1f;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "NeHe Lektion 03 - Farben", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glShadeModel(GL_SMOOTH); // Aktiviert weiche Farbübergänge
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glEnable(GL_DEPTH_TEST);

    int w, h; glfwGetFramebufferSize(window, &w, &h);
    framebuffer_size_callback(window, w, h);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        glTranslatef(-1.5f, 0.0f, -6.0f);
        glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.0f, 0.0f);    // Rot
            glVertex3f( 0.0f,  1.0f, 0.0f);
            glColor3f(0.0f, 1.0f, 0.0f);    // Grün
            glVertex3f(-1.0f, -1.0f, 0.0f);
            glColor3f(0.0f, 0.0f, 1.0f);    // Blau
            glVertex3f( 1.0f, -1.0f, 0.0f);
        glEnd();

        glTranslatef(3.0f, 0.0f, 0.0f);
        glColor3f(0.5f, 0.5f, 1.0f);        // Ein festes Hellblau
        glBegin(GL_QUADS);
            glVertex3f(-1.0f,  1.0f, 0.0f);
            glVertex3f( 1.0f,  1.0f, 0.0f);
            glVertex3f( 1.0f, -1.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, 0.0f);
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    }
    glfwTerminate();
    return 0;
}
