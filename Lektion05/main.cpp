#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>

float rtri = 0.0f; float rquad = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    float aspect = (float)width / (float)height;
    float fH = std::tan(45.0f / 360.0f * 3.14159f) * 0.1f;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "NeHe Lektion 05 - 3D Körper", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST); // WICHTIG: Erlaubt Objekten, sich gegenseitig zu verdecken
    glDepthFunc(GL_LEQUAL);

    int w, h; glfwGetFramebufferSize(window, &w, &h);
    framebuffer_size_callback(window, w, h);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // PYRAMIDE
        glTranslatef(-1.5f, 0.0f, -6.0f);
        glRotatef(rtri, 0.0f, 1.0f, 0.0f);
        glBegin(GL_TRIANGLES);
            // Vorne
            glColor3f(1.0f,0.0f,0.0f); glVertex3f( 0.0f, 1.0f, 0.0f);
            glColor3f(0.0f,1.0f,0.0f); glVertex3f(-1.0f,-1.0f, 1.0f);
            glColor3f(0.0f,0.0f,1.0f); glVertex3f( 1.0f,-1.0f, 1.0f);
            // Rechts
            glColor3f(1.0f,0.0f,0.0f); glVertex3f( 0.0f, 1.0f, 0.0f);
            glColor3f(0.0f,0.0f,1.0f); glVertex3f( 1.0f,-1.0f, 1.0f);
            glColor3f(0.0f,1.0f,0.0f); glVertex3f( 1.0f,-1.0f,-1.0f);
            // Hinten
            glColor3f(1.0f,0.0f,0.0f); glVertex3f( 0.0f, 1.0f, 0.0f);
            glColor3f(0.0f,1.0f,0.0f); glVertex3f( 1.0f,-1.0f,-1.0f);
            glColor3f(0.0f,0.0f,1.0f); glVertex3f(-1.0f,-1.0f,-1.0f);
            // Links
            glColor3f(1.0f,0.0f,0.0f); glVertex3f( 0.0f, 1.0f, 0.0f);
            glColor3f(0.0f,0.0f,1.0f); glVertex3f(-1.0f,-1.0f,-1.0f);
            glColor3f(0.0f,1.0f,0.0f); glVertex3f(-1.0f,-1.0f, 1.0f);
        glEnd();

        glLoadIdentity();

        // WÜRFEL
        glTranslatef(1.5f, 0.0f, -7.0f);
        glRotatef(rquad, 1.0f, 1.0f, 1.0f); // Drehung um alle Achsen
        glBegin(GL_QUADS);
            glColor3f(0.0f,1.0f,0.0f); // Oben
            glVertex3f( 1.0f, 1.0f,-1.0f); glVertex3f(-1.0f, 1.0f,-1.0f); glVertex3f(-1.0f, 1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 1.0f);
            glColor3f(1.0f,0.5f,0.0f); // Unten
            glVertex3f( 1.0f,-1.0f, 1.0f); glVertex3f(-1.0f,-1.0f, 1.0f); glVertex3f(-1.0f,-1.0f,-1.0f); glVertex3f( 1.0f,-1.0f,-1.0f);
            glColor3f(1.0f,0.0f,0.0f); // Vorne
            glVertex3f( 1.0f, 1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f); glVertex3f(-1.0f,-1.0f, 1.0f); glVertex3f( 1.0f,-1.0f, 1.0f);
            glColor3f(1.0f,1.0f,0.0f); // Hinten
            glVertex3f( 1.0f,-1.0f,-1.0f); glVertex3f(-1.0f,-1.0f,-1.0f); glVertex3f(-1.0f, 1.0f,-1.0f); glVertex3f( 1.0f, 1.0f,-1.0f);
            glColor3f(0.0f,0.0f,1.0f); // Links
            glVertex3f(-1.0f, 1.0f, 1.0f); glVertex3f(-1.0f, 1.0f,-1.0f); glVertex3f(-1.0f,-1.0f,-1.0f); glVertex3f(-1.0f,-1.0f, 1.0f);
            glColor3f(1.0f,0.0f,1.0f); // Rechts
            glVertex3f( 1.0f, 1.0f,-1.0f); glVertex3f( 1.0f, 1.0f, 1.0f); glVertex3f( 1.0f,-1.0f, 1.0f); glVertex3f( 1.0f,-1.0f,-1.0f);
        glEnd();

        rtri += 0.2f; rquad -= 0.15f;
        glfwSwapBuffers(window); glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    }
    glfwTerminate(); return 0;
}
