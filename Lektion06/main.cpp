#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <vector>

float xrot = 0.0f; float yrot = 0.0f; GLuint texture;

void createTexture() {
    const int w = 64, h = 64;
    std::vector<GLubyte> img(w * h * 3);
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++) {
            GLubyte c = ((((i & 0x4) == 0) ^ ((j & 0x4) == 0))) * 255;
            int idx = (i * w + j) * 3;
            img[idx] = c; img[idx+1] = c/2; img[idx+2] = 0;
        }
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
}

void setupProjection(int width, int height) {
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Lektion 06 - Texturierter Würfel", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glEnable(GL_TEXTURE_2D); glEnable(GL_DEPTH_TEST);

    int w, h; glfwGetFramebufferSize(window, &w, &h);
    setupProjection(w, h);
    createTexture();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef(xrot, 1.0f, 0.0f, 0.0f); glRotatef(yrot, 0.0f, 1.0f, 0.0f);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        glEnd();
        xrot += 0.3f; yrot += 0.2f;
        glfwSwapBuffers(window); glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }
    glfwTerminate(); return 0;
}
