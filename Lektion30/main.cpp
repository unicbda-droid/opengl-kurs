#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <iostream>

// Kugel-Variablen
float ballX = 0.0f, ballY = 1.5f, ballZ = -4.0f;
float ballRadius = 0.3f;
float velY = -0.03f; // Fallgeschwindigkeit (Schwerkraft-Simulation)

// Boden-Variable (Y-Koordinate des Bodens)
float floorY = -1.0f;

void drawBall() {
    glPushMatrix();
    glTranslatef(ballX, ballY, ballZ);
    glColor3f(1.0f, 0.3f, 0.3f);
    
    // Wir zeichnen eine einfache Kugel aus Linien
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<32; i++) {
        float angle = i * 2.0f * 3.14159265f / 32.0f;
        glVertex3f(std::cos(angle)*ballRadius, std::sin(angle)*ballRadius, 0.0f);
    }
    glEnd();
    glPopMatrix();
}

void drawFloor() {
    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f);
    glVertex3f(-2.0f, floorY, -6.0f);
    glVertex3f( 2.0f, floorY, -6.0f);
    glVertex3f( 2.0f, floorY, -2.0f);
    glVertex3f(-2.0f, floorY, -2.0f);
    glEnd();
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 30 - Collision Detection", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // --- PHYSIK & KOLLISIONSABFRAGE ---
        ballY += velY; // Kugel bewegen

        // Mathematischer Check: Ist der Abstand zum Boden kleiner als der Radius?
        if (ballY - ballRadius <= floorY) {
            ballY = floorY + ballRadius; // Kugel exakt auf den Boden setzen (Eindringen verhindern)
            velY = -velY; // Vektor invertieren -> Abprall-Effekt!
            std::cout << "Kollision detektiert! Kugel prallt ab." << std::endl;
        }

        // Schwerkraft simulieren (beschleunigt den Fall leicht)
        velY -= 0.001f;

        // Zeichnen
        drawFloor();
        drawBall();

        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}