#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
float rX=0.0f, rY=0.0f;
void drawSeg(float x, float y, float z, float sx, float sy, float sz) {
    glPushMatrix(); glTranslatef(x,y,z); glScalef(sx,sy,sz);
    glBegin(GL_QUADS);
    glNormal3f(0.0f,0.0f,1.0f); glVertex3f(-0.5f,-0.5f,0.5f); glVertex3f(0.5f,-0.5f,0.5f); glVertex3f(0.5f,0.5f,0.5f); glVertex3f(-0.5f,0.5f,0.5f);
    glNormal3f(0.0f,0.0f,-1.0f); glVertex3f(-0.5f,-0.5f,-0.5f); glVertex3f(-0.5f,0.5f,-0.5f); glVertex3f(0.5f,0.5f,-0.5f); glVertex3f(0.5f,-0.5f,-0.5f);
    glEnd(); glPopMatrix();
}
int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 14 - 3D Vector Fonts", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL);
    GLfloat lp[]={3.0f,3.0f,5.0f,1.0f}; glLightfv(GL_LIGHT0, GL_POSITION, lp);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); glLoadIdentity();
        glTranslatef(0.0f,0.0f,-8.0f); glRotatef(rX,1.0f,0.0f,0.0f); glRotatef(rY,0.0f,1.0f,0.0f);
        glColor3f(0.0f,0.6f,1.0f); drawSeg(0.0f,1.0f,0.0f,1.0f,0.2f,0.4f); drawSeg(0.4f,0.5f,0.0f,0.2f,1.0f,0.4f); drawSeg(0.0f,0.0f,0.0f,1.0f,0.2f,0.4f);
        rX+=0.4f; rY+=0.6f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}