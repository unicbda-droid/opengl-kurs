#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>

float rY = 0.0f;

GLfloat cubeVertices[] = {
    -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f
};

GLfloat cubeColors[] = {
    1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,
    0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,
    0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,
    1.0f,1.0f,0.0f,  1.0f,1.0f,0.0f,  1.0f,1.0f,0.0f,  1.0f,1.0f,0.0f,
    0.0f,1.0f,1.0f,  0.0f,1.0f,1.0f,  0.0f,1.0f,1.0f,  0.0f,1.0f,1.0f,
    1.0f,0.0f,1.0f,  1.0f,0.0f,1.0f,  1.0f,0.0f,1.0f,  1.0f,0.0f,1.0f
};

void drawCube() {
    glDrawArrays(GL_QUADS, 0, 24);
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 36 - Radial Blur Effect", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w); 
    glEnable(GL_DEPTH_TEST);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, cubeVertices);
    glColorPointer(3, GL_FLOAT, 0, cubeColors);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 1. Zuerst den scharfen Hauptwuerfel in der Mitte zeichnen
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -4.0f);
        glRotatef(rY, 1.0f, 0.7f, 0.3f);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        drawCube();

        // 2. JETZT DIE BLUR-MAGIE: Wir zeichnen den Wuerfel 10-mal transluzent darueber
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additives Blending (leuchtet genial!)
        glDisable(GL_DEPTH_TEST);         // Tiefentest aus, damit die Schichten verschmelzen

        float blurFactor = 1.0f;
        for(int i = 0; i < 10; i++) {
            glLoadIdentity(); 
            glTranslatef(0.0f, 0.0f, -4.0f);
            
            // Jede Schicht wird ein winziges Stueckchen staerker skaliert (zieht den Effekt nach aussen)
            glScalef(blurFactor, blurFactor, blurFactor); 
            glRotatef(rY, 1.0f, 0.7f, 0.3f);
            
            // Je weiter aussen die Schicht, desto blasser wird sie
            glColor4f(1.0f, 1.0f, 1.0f, 0.08f); 
            drawCube();
            
            blurFactor += 0.015f; // Erhoehe den "Auszug" des Blurs
        }

        rY += 1.5f; // Schnelle Rotation, damit der Blur richtig knallt!
        glfwSwapBuffers(w); glfwPollEvents();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glfwTerminate(); return 0;
}
