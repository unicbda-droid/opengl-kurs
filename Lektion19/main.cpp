#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <cstdlib>
#include <cmath>

const int MAX_PARTICLES = 1000;

struct Particle {
    bool active;
    float life;
    float fade;
    float r, g, b;
    float x, y, z;
    float xi, yi, zi; // Geschwindigkeit / Richtung
};

std::vector<Particle> particles(MAX_PARTICLES);

void initParticle(int i) {
    particles[i].active = true;
    particles[i].life = 1.0f;
    // Zufällige Sterbegeschwindigkeit
    particles[i].fade = float(rand() % 100) / 1000.0f + 0.003f;
    
    // Startposition im Ursprung
    particles[i].x = 0.0f; particles[i].y = -1.5f; particles[i].z = 0.0f;
    
    // Zufällige Richtung nach oben (wie eine Stichflamme)
    particles[i].xi = float((rand() % 50) - 25) / 500.0f;
    particles[i].yi = float(rand() % 100) / 500.0f + 0.05f;
    particles[i].zi = float((rand() % 50) - 25) / 500.0f;
    
    // Feuer-Farben: Viel Rot, etwas Grün, kein Blau
    particles[i].r = 1.0f;
    particles[i].g = float(rand() % 50) / 100.0f;
    particles[i].b = 0.0f;
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 19 - Partikelsystem (Feuereffekt)", NULL, NULL);
    glfwMakeContextCurrent(w);
    
    // Blending für den Leuchteffekt aktivieren (Transparenz-Addierung)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST); // Aus für besseres Partikel-Overlapping

    for(int i=0; i<MAX_PARTICLES; i++) initParticle(i);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); glClear(GL_COLOR_BUFFER_BIT);
        
        for(int i=0; i<MAX_PARTICLES; i++) {
            if (particles[i].active) {
                glLoadIdentity(); glTranslatef(0.0f, 0.0f, -6.0f);
                
                // Partikel als kleines leuchtendes Viereck zeichnen
                // Alpha-Wert ist an das verbleibende Leben gekoppelt
                glColor4f(particles[i].r, particles[i].g, particles[i].b, particles[i].life);
                
                float x = particles[i].x; float y = particles[i].y; float z = particles[i].z;
                float size = 0.04f;
                glBegin(GL_TRIANGLE_STRIP);
                    glVertex3f(x + size, y + size, z);
                    glVertex3f(x - size, y + size, z);
                    glVertex3f(x + size, y - size, z);
                    glVertex3f(x - size, y - size, z);
                glEnd();

                // Bewegung updaten
                particles[i].x += particles[i].xi;
                particles[i].y += particles[i].yi;
                particles[i].z += particles[i].zi;

                // Schwerkraft / Wind simulieren (leichtes Abdriften nach links)
                particles[i].xi -= 0.0005f;
                
                // Lebensenergie abziehen
                particles[i].life -= particles[i].fade;

                // Wenn tot, dann wiederbeleben
                if (particles[i].life < 0.0f) initParticle(i);
            }
        }

        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}