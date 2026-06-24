#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <iostream>
#include <vector>

bool light = true;
bool lp = false;
bool blend = true;
bool bp = false;

float xrot = 0.0f;
float yrot = 0.0f;
float xspeed = 0.2f;
float yspeed = 0.2f;
float z = -6.0f;

GLuint textureID;

GLfloat LightAmbient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat LightDiffuse[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

void createProceduralTexture() {
    const int width = 64;
    const int height = 64;
    std::vector<GLubyte> image(width * height * 3);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            GLubyte c = ((((i & 0x8) == 0) ^ ((j & 0x8) == 0))) * 255;
            int index = (i * width + j) * 3;
            image[index]     = c;       
            image[index + 1] = c / 2;   
            image[index + 2] = 0;       
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
    
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (!lp) {
            lp = true;
            light = !light;
            if (!light) glDisable(GL_LIGHTING); else glEnable(GL_LIGHTING);
        }
    } else { lp = false; }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        if (!bp) {
            bp = true;
            blend = !blend;
            if (blend) {
                glEnable(GL_BLEND);
                glDisable(GL_DEPTH_TEST);
            } else {
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
            }
        }
    } else { bp = false; }

    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)   z -= 0.05f;
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) z += 0.05f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)        xspeed -= 0.02f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)      xspeed += 0.02f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)     yspeed += 0.02f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)      yspeed -= 0.02f;
}

int main() {
    if (!glfwInit()) return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(800, 600, "NeHe Lektion 08 - Blending", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    
    glfwSwapInterval(1);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    createProceduralTexture();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (blend) {
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
    } else {
        glEnable(GL_DEPTH_TEST);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLfloat aspect = 800.0f / 600.0f;
    GLfloat fFOV = 45.0f * 3.14159265f / 180.0f;
    GLfloat fH = std::tan(fFOV / 2.0f) * 0.1f;
    GLfloat fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

    while (!glfwWindowShouldClose(window)) {
        handleInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        glTranslatef(0.0f, 0.0f, z);
        glRotatef(xrot, 1.0f, 0.0f, 0.0f);
        glRotatef(yrot, 0.0f, 1.0f, 0.0f);

        glBindTexture(GL_TEXTURE_2D, textureID);

        GLfloat mat_ambient_diffuse[] = { 1.0f, 1.0f, 1.0f, 0.4f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambient_diffuse);

        glBegin(GL_QUADS);
            // Front
            glNormal3f(0.0f, 0.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            // Back
            glNormal3f(0.0f, 0.0f, -1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            // Top
            glNormal3f(0.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
            // Bottom
            glNormal3f(0.0f, -1.0f, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            // Right
            glNormal3f(1.0f, 0.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            // Left
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glEnd();

        xrot += xspeed;
        yrot += yspeed;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteTextures(1, &textureID);
    glfwTerminate();
    return 0;
}