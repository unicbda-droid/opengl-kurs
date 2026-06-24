#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
float rX=0.0f, rY=0.0f;
// Drei verschiedene Nebel-Modi von OpenGL (Linear, Exponential, Exponential Squared)
GLint fogMode[] = { GL_EXP, GL_EXP2, GL_LINEAR };
GLuint fogFilter = 2; // Wir nutzen direkt den linearen Nebel
GLfloat fogColor[4] = {0.1f, 0.1f, 0.1f, 1.0f}; // Dunkelgrauer Nebel

void drawCube() {
    glBegin(GL_QUADS);
    glNormal3f(0.0f,0.0f,1.0f); glVertex3f(-1.0f,-1.0f,1.0f); glVertex3f(1.0f,-1.0f,1.0f); glVertex3f(1.0f,1.0f,1.0f); glVertex3f(-1.0f,1.0f,1.0f);
    glNormal3f(0.0f,0.0f,-1.0f); glVertex3f(-1.0f,-1.0f,-1.0f); glVertex3f(-1.0f,1.0f,-1.0f); glVertex3f(1.0f,1.0f,-1.0f); glVertex3f(1.0f,-1.0f,-1.0f);
    glNormal3f(0.0f,1.0f,0.0f); glVertex3f(-1.0f,1.0f,-1.0f); glVertex3f(-1.0f,1.0f,1.0f); glVertex3f(1.0f,1.0f,1.0f); glVertex3f(1.0f,1.0f,-1.0f);
    glNormal3f(0.0f,-1.0f,0.0f); glVertex3f(-1.0f,-1.0f,-1.0f); glVertex3f(1.0f,-1.0f,-1.0f); glVertex3f(1.0f,-1.0f,1.0f); glVertex3f(-1.0f,-1.0f,1.0f);
    glEnd();
}
int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 16 - Nebeleffekte", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL);
    GLfloat lp[]={3.0f,3.0f,5.0f,1.0f}; glLightfv(GL_LIGHT0, GL_POSITION, lp);
    
    // Nebel-Setup
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, fogMode[fogFilter]);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.35f);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf(GL_FOG_START, 3.0f);  // Wo der Nebel anfängt
    glFogf(GL_FOG_END, 12.0f);    // Wo alles komplett im Nebel verschwindet
    
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        // Hintergrundfarbe muss der Nebelfarbe entsprechen, sonst sieht es falsch aus!
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Würfel 1: Nah an der Kamera (wenig Nebel)
        glLoadIdentity(); glTranslatef(-1.5f,0.0f,-4.0f); glRotatef(rX,1.0f,0.0f,0.0f); glColor3f(0.0f,0.6f,1.0f); drawCube();
        
        // Würfel 2: Weiter hinten im Raum (verschwindet fast im Nebel)
        glLoadIdentity(); glTranslatef(1.5f,0.0f,-10.0f); glRotatef(rY,0.0f,1.0f,0.0f); glColor3f(1.0f,0.3f,0.0f); drawCube();
        
        rX+=0.4f; rY+=0.6f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}