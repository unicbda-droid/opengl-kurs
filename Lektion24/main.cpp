#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>
#include <string>
#include <cmath>

float rX = 0.0f;

void checkExtensions() {
    const GLubyte* ext = glGetString(GL_EXTENSIONS);
    std::cout << "=== GRAFIKKARTEN INFOS ===" << std::endl;
    std::cout << "Vendor:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version:  " << glGetString(GL_VERSION) << std::endl;
    
    if (ext) {
        std::string extStr(reinterpret_cast<const char*>(ext));
        std::cout << std::endl << "Unterstuetzeit Scissor Test: ";
        if (extStr.find("GL_ARB_multitexture") != std::string::npos) {
            std::cout << "JA (und Multitexturing auch!)" << std::endl;
        } else {
            std::cout << "JA (Standard Feature)" << std::endl;
        }
    }
    std::cout << "==========================" << std::endl;
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 24 - Extensions & Scissor", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);
    
    checkExtensions();

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glEnable(GL_SCISSOR_TEST);
        glScissor(200, 150, 400, 300);
        
        glClearColor(0.0f, 0.5f, 0.7f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -3.5f);
        glRotatef(rX, 1.0f, 1.0f, 0.0f);

        glBegin(GL_QUADS);
            glColor3f(1.0f, 1.0f, 0.0f);
            glVertex3f(-1.5f, -1.5f, 0.0f);
            glVertex3f( 1.5f, -1.5f, 0.0f);
            glVertex3f( 1.5f,  1.5f, 0.0f);
            glVertex3f(-1.5f,  1.5f, 0.0f);
        glEnd();

        glDisable(GL_SCISSOR_TEST);

        rX += 1.0f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}