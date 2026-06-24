#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <cmath>

float playerX = 0.0f; float playerY = 0.0f;
float enemyX = 5.0f; float enemyY = 4.0f;
float cnt = 0.0f;

void drawGrid() {
    // Antialiasing für Linien aktivieren, damit sie nicht "treppig" aussehen
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    glColor3f(0.0f, 0.5f, 1.0f); // Blaues Raster
    // Vertikale Linien
    for(float x = -10.0f; x <= 10.0f; x += 1.0f) {
        glVertex2f(x, -10.0f); glVertex2f(x, 10.0f);
    }
    // Horizontale Linien
    for(float y = -10.0f; y <= 10.0f; y += 1.0f) {
        glVertex2f(-10.0f, y); glVertex2f(10.0f, y);
    }
    glEnd();
    glDisable(GL_LINE_SMOOTH);
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 21 - Linien & Rasterlogik", NULL, NULL);
    glfwMakeContextCurrent(w);
    
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(-11.0, 11.0, -11.0, 11.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    double lastTime = glfwGetTime();

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        // Einfache Steuerung des Spielers über die Pfeiltasten
        if(glfwGetKey(w, GLFW_KEY_LEFT)==GLFW_PRESS)  playerX -= 5.0f * deltaTime;
        if(glfwGetKey(w, GLFW_KEY_RIGHT)==GLFW_PRESS) playerX += 5.0f * deltaTime;
        if(glfwGetKey(w, GLFW_KEY_UP)==GLFW_PRESS)    playerY += 5.0f * deltaTime;
        if(glfwGetKey(w, GLFW_KEY_DOWN)==GLFW_PRESS)  playerY -= 5.0f * deltaTime;

        // Der Gegner verfolgt den Spieler stumpf auf dem Raster
        if(enemyX < playerX) enemyX += 2.0f * deltaTime;
        if(enemyX > playerX) enemyX -= 2.0f * deltaTime;
        if(enemyY < playerY) enemyY += 2.0f * deltaTime;
        if(enemyY > playerY) enemyY -= 2.0f * deltaTime;

        glClearColor(0.0f, 0.05f, 0.1f, 1.0f); glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();

        // 1. Das Spielfeld zeichnen
        drawGrid();

        // 2. Den Spieler zeichnen (Grünes Quadrat)
        glLoadIdentity(); glTranslatef(playerX, playerY, 0.0f);
        glBegin(GL_QUADS);
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex2f(-0.4f, -0.4f); glVertex2f(0.4f, -0.4f);
            glVertex2f(0.4f, 0.4f);   glVertex2f(-0.4f, 0.4f);
        glEnd();

        // 3. Den Gegner zeichnen (Rotes pulsierendes Quadrat)
        glLoadIdentity(); glTranslatef(enemyX, enemyY, 0.0f);
        float size = 0.4f + 0.1f * std::sin(cnt);
        glBegin(GL_QUADS);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2f(-size, -size); glVertex2f(size, -size);
            glVertex2f(size, size);   glVertex2f(-size, size);
        glEnd();

        cnt += 0.1f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}