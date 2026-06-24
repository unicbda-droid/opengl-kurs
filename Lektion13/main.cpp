#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <string>
float c1=0.0f, c2=0.0f;
const unsigned char font[4][8] = {
    {0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    {0x3C, 0x66, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00},
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}
};
void drawChar(char c) {
    int idx = (c=='A')?0:(c=='B')?1:(c=='C')?2:(c=='I')?3:-1;
    if(idx!=-1) glBitmap(8, 8, 0.0f, 0.0f, 10.0f, 0.0f, font[idx]);
    else glBitmap(0, 0, 0.0f, 0.0f, 8.0f, 0.0f, nullptr);
}
int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 13 - Bitmap Fonts", NULL, NULL);
    glfwMakeContextCurrent(w);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -10.0f); glColor3f(1.0f, 0.5f+0.5f*std::sin(c1), 0.0f);
        glRasterPos3f(-2.0f+0.5f*std::cos(c1), 1.0f+0.3f*std::sin(c2), 0.0f);
        for(char ch : std::string("ABC BA")) drawChar(ch);
        c1+=0.05f; c2+=0.04f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}