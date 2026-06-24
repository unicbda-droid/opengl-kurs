#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <vector>
float rX=0.0f, rY=0.0f;
GLuint textureID;
void createTexture() {
    std::vector<GLubyte> img(64*64*3);
    for(int i=0; i<64; i++) {
        for(int j=0; j<64; j++) {
            GLubyte c = ((((i&0x4)==0)^((j&0x4)==0)))*180+50;
            int idx = (i*64+j)*3;
            img[idx]=c; img[idx+1]=c; img[idx+2]=255;
        }
    }
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
}
void drawSeg(float x, float y, float z, float sx, float sy, float sz) {
    glPushMatrix(); glTranslatef(x,y,z); glScalef(sx,sy,sz);
    glBegin(GL_QUADS);
    glNormal3f(0.0f,0.0f,1.0f);
    glTexCoord2f(0.0f,0.0f); glVertex3f(-0.5f,-0.5f,0.5f); glTexCoord2f(1.0f,0.0f); glVertex3f(0.5f,-0.5f,0.5f); glTexCoord2f(1.0f,1.0f); glVertex3f(0.5f,0.5f,0.5f); glTexCoord2f(0.0f,1.0f); glVertex3f(-0.5f,0.5f,0.5f);
    glNormal3f(0.0f,0.0f,-1.0f);
    glTexCoord2f(1.0f,0.0f); glVertex3f(-0.5f,-0.5f,-0.5f); glTexCoord2f(1.0f,1.0f); glVertex3f(-0.5f,0.5f,-0.5f); glTexCoord2f(0.0f,1.0f); glVertex3f(0.5f,0.5f,-0.5f); glTexCoord2f(0.0f,0.0f); glVertex3f(0.5f,-0.5f,-0.5f);
    glEnd(); glPopMatrix();
}
int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 15 - Textured 3D Fonts", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL); glEnable(GL_TEXTURE_2D);
    GLfloat lp[]={3.0f,3.0f,5.0f,1.0f}; glLightfv(GL_LIGHT0, GL_POSITION, lp); createTexture();
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); glLoadIdentity();
        glTranslatef(0.0f,0.0f,-8.0f); glRotatef(rX,1.0f,0.0f,0.0f); glRotatef(rY,0.0f,1.0f,0.0f);
        glBindTexture(GL_TEXTURE_2D, textureID); glColor3f(1.0f,1.0f,1.0f);
        drawSeg(0.0f,1.0f,0.0f,1.0f,0.2f,0.4f); drawSeg(0.4f,0.5f,0.0f,0.2f,1.0f,0.4f); drawSeg(0.0f,0.0f,0.0f,1.0f,0.2f,0.4f);
        rX+=0.4f; rY+=0.6f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}