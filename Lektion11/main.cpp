#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <vector>
const int GRID_SIZE = 45;
float points[GRID_SIZE][GRID_SIZE][3];
float hold;
float xrot = 0.0f, yrot = 0.0f, zrot = 0.0f;
float xspeed = 0.1f, yspeed = 0.1f;
GLuint textureID;
void createTexture() {
    std::vector<GLubyte> img(64*64*3);
    for(int i=0; i<64; i++) {
        for(int j=0; j<64; j++) {
            GLubyte c = (((i&0x8)==0)||((j&0x8)==0))?255:50;
            int idx = (i*64+j)*3;
            img[idx]=c; img[idx+1]=c/4; img[idx+2]=0;
        }
    }
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
}
int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 11 - Welle", NULL, NULL);
    glfwMakeContextCurrent(w);
    glEnable(GL_DEPTH_TEST); glEnable(GL_TEXTURE_2D); createTexture();
    for(int x=0; x<GRID_SIZE; x++) {
        for(int y=0; y<GRID_SIZE; y++) {
            points[x][y][0]=(float(x)/5.0f)-4.5f;
            points[x][y][1]=(float(y)/5.0f)-4.5f;
            points[x][y][2]=std::sin((((float(x)/5.0f)*40.0f)/360.0f)*3.14159265f*2.0f);
        }
    }
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    int cycle = 0;
    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -12.0f);
        glRotatef(xrot, 1.0f, 0.0f, 0.0f); glRotatef(yrot, 0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        for(int x=0; x<GRID_SIZE-1; x++) {
            for(int y=0; y<GRID_SIZE-1; y++) {
                float fx = float(x)/(GRID_SIZE-1), fy = float(y)/(GRID_SIZE-1);
                float fxb = float(x+1)/(GRID_SIZE-1), fyb = float(y+1)/(GRID_SIZE-1);
                glTexCoord2f(fx, fy); glVertex3f(points[x][y][0], points[x][y][1], points[x][y][2]);
                glTexCoord2f(fx, fyb); glVertex3f(points[x][y+1][0], points[x][y+1][1], points[x][y+1][2]);
                glTexCoord2f(fxb, fyb); glVertex3f(points[x+1][y+1][0], points[x+1][y+1][1], points[x+1][y+1][2]);
                glTexCoord2f(fxb, fy); glVertex3f(points[x+1][y][0], points[x+1][y][1], points[x+1][y][2]);
            }
        }
        glEnd();
        if(cycle >= 2) {
            for(int y=0; y<GRID_SIZE; y++) {
                hold = points[0][y][2];
                for(int x=0; x<GRID_SIZE-1; x++) points[x][y][2] = points[x+1][y][2];
                points[GRID_SIZE-1][y][2] = hold;
            }
            cycle = 0;
        }
        cycle++; xrot+=xspeed; yrot+=yspeed;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}