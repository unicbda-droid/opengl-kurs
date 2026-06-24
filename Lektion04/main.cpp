#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>

float rtri = 0.0f;  // Winkel für das Dreieck
float rquad = 0.0f; // Winkel für das Quadrat

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
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "NeHe Lektion 04 - Rotation", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    int w, h; glfwGetFramebufferSize(window, &w, &h);
    framebuffer_size_callback(window, w, h);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // DREIECK: Rotiert um die Y-Achse (vertikal)
        glTranslatef(-1.5f, 0.0f, -6.0f);
        glRotatef(rtri, 0.0f, 1.0f, 0.0f); // (Winkel, X, Y, Z)
        glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.0f, 0.0f); glVertex3f( 0.0f,  1.0f, 0.0f);
            glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
            glColor3f(0.0f, 0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
        glEnd();

        glLoadIdentity(); // Wichtig: Zurücksetzen für das nächste Objekt!

        // QUADRAT: Rotiert um die X-Achse (horizontal)
        glTranslatef(1.5f, 0.0f, -6.0f);
        glRotatef(rquad, 1.0f, 0.0f, 0.0f);
        glColor3f(0.5f, 0.5f, 1.0f);
        glBegin(GL_QUADS);
            glVertex3f(-1.0f,  1.0f, 0.0f);
            glVertex3f( 1.0f,  1.0f, 0.0f);
            glVertex3f( 1.0f, -1.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, 0.0f);
        glEnd();

        // Werte erhöhen für die nächste Runde
        rtri  += 0.2f;
        rquad -= 0.15f;

        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    }
    glfwTerminate();
    return 0;
}
