#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <vector>
#include <iostream>

float xpos = 0.0f, zpos = 0.0f, yrot = 0.0f, lookupdown = 0.0f;
const float piover180 = 0.0174532925f;
GLuint textureID;

struct Vertex { float x, y, z, u, v; };
std::vector<Vertex> worldSector;

void buildWorld() {
    Vertex v[] = {
        {-10,-1,-10, 0,10}, { 10,-1,-10,10,10}, { 10,-1, 10,10, 0}, {-10,-1, 10, 0, 0},
        {-10, 1,-10, 0,10}, {-10, 1, 10, 0, 0}, { 10, 1, 10,10, 0}, { 10, 1,-10,10,10},
        {-10, 1,-10, 0, 1}, { 10, 1,-10, 4, 1}, { 10,-1,-10, 4, 0}, {-10,-1,-10, 0, 0},
        {-10, 1, 10, 0, 1}, {-10,-1, 10, 0, 0}, { 10,-1, 10, 4, 0}, { 10, 1, 10, 4, 1},
        {-10, 1, 10, 4, 1}, {-10, 1,-10, 0, 1}, {-10,-1,-10, 0, 0}, {-10,-1, 10, 4, 0},
        { 10, 1, 10, 4, 1}, { 10,-1, 10, 4, 0}, { 10,-1,-10, 0, 0}, { 10, 1,-10, 0, 1}
    };
    worldSector.assign(v, v + sizeof(v)/sizeof(Vertex));
}

void createTexture() {
    int w=64, h=64; std::vector<GLubyte> img(w*h*3);
    for(int i=0;i<h;i++) for(int j=0;j<w;j++) {
        GLubyte c = ((((i&0x4)==0)^((j&0x4)==0)))*255;
        int idx=(i*w+j)*3; img[idx]=c*0.3f; img[idx+1]=c*0.6f; img[idx+2]=c;
    }
    glGenTextures(1,&textureID); glBindTexture(GL_TEXTURE_2D,textureID);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0,GL_RGB,GL_UNSIGNED_BYTE,img.data());
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800,600,"NeHe Lektion 10 - 3D-Welt",NULL,NULL);
    if(!w){glfwTerminate();return -1;}
    glfwMakeContextCurrent(w); glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST); glEnable(GL_TEXTURE_2D);
    createTexture(); buildWorld();
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    float a=800.f/600.f, fFOV=45.f*3.14159f/180.f;
    float fH=tan(fFOV/2.f)*0.1f, fW=fH*a;
    glFrustum(-fW,fW,-fH,fH,0.1f,100.f);
    glMatrixMode(GL_MODELVIEW);
    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w,GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(w,true);
        float sp=0.05f;
        if(glfwGetKey(w,GLFW_KEY_RIGHT)==GLFW_PRESS) yrot-=1.5f;
        if(glfwGetKey(w,GLFW_KEY_LEFT)==GLFW_PRESS) yrot+=1.5f;
        if(glfwGetKey(w,GLFW_KEY_UP)==GLFW_PRESS){xpos-=sin(yrot*piover180)*sp;zpos-=cos(yrot*piover180)*sp;}
        if(glfwGetKey(w,GLFW_KEY_DOWN)==GLFW_PRESS){xpos+=sin(yrot*piover180)*sp;zpos+=cos(yrot*piover180)*sp;}
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); glLoadIdentity();
        glRotatef(lookupdown,1,0,0); glRotatef(360-yrot,0,1,0); glTranslatef(-xpos,0,-zpos);
        glBindTexture(GL_TEXTURE_2D,textureID);
        glBegin(GL_QUADS); for(auto& v:worldSector){glTexCoord2f(v.u,v.v);glVertex3f(v.x,v.y,v.z);} glEnd();
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glDeleteTextures(1,&textureID); glfwTerminate(); return 0;
}
