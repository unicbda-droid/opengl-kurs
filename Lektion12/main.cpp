#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <vector>
GLuint boxList, textureID;
float xrot=0.0f, yrot=0.0f;
void createTexture() {
    std::vector<GLubyte> img(64*64*3);
    for(int i=0; i<64; i++) {
        for(int j=0; j<64; j++) {
            GLubyte c = ((((i&0x8)==0)^((j&0x8)==0)))*150+100;
            int idx = (i*64+j)*3;
            img[idx]=c; img[idx+1]=c*0.5f; img[idx+2]=20;
        }
    }
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
}
void buildList() {
    boxList = glGenLists(1); glNewList(boxList, GL_COMPILE);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
    glEnd(); glEndList();
}
int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 12 - Display Listen", NULL, NULL);
    glfwMakeContextCurrent(w);
    glEnable(GL_DEPTH_TEST); glEnable(GL_TEXTURE_2D); createTexture(); buildList();
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for(int x=-2; x<=2; x++) {
            for(int y=-2; y<=2; y++) {
                glLoadIdentity(); glTranslatef(float(x)*3.0f, float(y)*3.0f, -15.0f);
                glRotatef(xrot, 1.0f, 0.0f, 0.0f); glRotatef(yrot, 0.0f, 1.0f, 0.0f);
                glCallList(boxList);
            }
        }
        xrot+=0.5f; yrot+=0.5f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glDeleteLists(boxList, 1); glfwTerminate(); return 0;
}