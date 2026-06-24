#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>

float rY = 0.0f;

// NUR NOCH 8 ECKEN! Kein einziger Punkt ist doppelt.
GLfloat cubeVertices[] = {
    -1.0f, -1.0f,  1.0f,  // 0
     1.0f, -1.0f,  1.0f,  // 1
     1.0f,  1.0f,  1.0f,  // 2
    -1.0f,  1.0f,  1.0f,  // 3
    -1.0f, -1.0f, -1.0f,  // 4
     1.0f, -1.0f, -1.0f,  // 5
     1.0f,  1.0f, -1.0f,  // 6
    -1.0f,  1.0f, -1.0f   // 7
};

// Reine Farbinformationen für die 8 Ecken
GLfloat cubeColors[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f
};

// HIER IST DER INDEX-BUFFER: Welche Ecken bilden welche Wand?
GLubyte cubeIndices[] = {
    0, 1, 2, 3,  // Vorne
    4, 5, 6, 7,  // Hinten
    4, 0, 3, 7,  // Links
    1, 5, 6, 2,  // Rechts
    3, 2, 6, 7,  // Oben
    4, 5, 1, 0   // Unten
};

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 34 - Indexed Vertex Arrays", NULL, NULL);
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
        glClearColor(0.08f, 0.05f, 0.05f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -4.0f);
        glRotatef(rY, 1.0f, 1.0f, 0.5f);

        // JETZT MIT NEUEM BEFEHL: glDrawElements statt glDrawArrays
        // Wir übergeben die Anzahl der Indizes (24) und das Index-Array
        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, cubeIndices);

        rY += 0.5f;
        glfwSwapBuffers(w); glfwPollEvents();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glfwTerminate(); return 0;
}
