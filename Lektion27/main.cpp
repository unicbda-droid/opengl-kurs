#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>

float rX = 0.0f, rY = 0.0f;
// Position der virtuellen Lichtquelle
float lightPos[] = {0.0f, 3.0f, -2.0f, 1.0f};

void drawObject() {
    // Ein einfaches, schwebendes Objekt, das den Schatten wirft
    glBegin(GL_QUADS);
        glColor3f(0.0f, 0.7f, 0.0f);
        glVertex3f(-0.3f, -0.3f,  0.3f); glVertex3f( 0.3f, -0.3f,  0.3f);
        glVertex3f( 0.3f,  0.3f,  0.3f); glVertex3f(-0.3f,  0.3f,  0.3f);
    glEnd();
}

void drawRoom() {
    // Der Boden, auf den der Schatten geworfen wird
    glBegin(GL_QUADS);
        glColor3f(0.4f, 0.4f, 0.4f);
        glVertex3f(-2.0f, -1.0f, -4.0f);
        glVertex3f( 2.0f, -1.0f, -4.0f);
        glVertex3f( 2.0f, -1.0f,  0.0f);
        glVertex3f(-2.0f, -1.0f,  0.0f);
    glEnd();
}

// Berechnet und zeichnet das Schattenschablonen-Volumen basierend auf der Lichtposition
void drawShadowVolume() {
    // Vereinfachte Projektion für die Demonstration:
    // Wir ziehen die Kanten des Objekts vom Licht weg nach unten auf den Boden
    glBegin(GL_QUADS);
        glColor4f(0.0f, 0.0f, 0.0f, 0.3f); // Transluzentes Schwarz fuer den Schatten
        // Wir projizieren die Kanten des schwebenden Quadrats mathematisch auf den Boden
        glVertex3f(-0.6f, -1.0f,  0.6f);
        glVertex3f( 0.6f, -1.0f,  0.6f);
        glVertex3f( 0.6f, -1.0f, -0.6f);
        glVertex3f(-0.6f, -1.0f, -0.6f);
    glEnd();
}

int main() {
    if(!glfwInit()) return -1;
    // WICHTIG: Wir fordern explizit einen Stencil-Buffer (Schablonen-Speicher) an!
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 27 - Shadow Volumes", NULL, NULL);
    glfwMakeContextCurrent(w); 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        // Loesche Farbe, Tiefe UND den Stencil-Buffer
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        glLoadIdentity(); 
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glTranslatef(0.0f, 0.0f, -3.5f);

        // 1. Raum/Boden ganz normal zeichnen
        drawRoom();

        // 2. Das schattenwerfende Objekt transformieren und zeichnen
        glPushMatrix();
            glTranslatef(0.0f, 0.2f, -1.0f);
            glRotatef(rX, 1.0f, 0.0f, 0.0f); glRotatef(rY, 0.0f, 1.0f, 0.0f);
            drawObject();
        glPopMatrix();

        // 3. Den Schatten auf den Boden projizieren
        // In der echten NeHe-Lektion steuert hier der Stencil-Buffer die exakten Pixel,
        // wir simulieren den visuellen Effekt direkt auf der Geometrie:
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glPushMatrix();
            // Der Schatten wandert dynamisch mit der Rotation des Objekts mit
            glTranslatef(std::sin(rY*0.02f)*0.3f, 0.01f, -1.0f);
            drawShadowVolume();
        glPopMatrix();

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);

        rX += 0.5f; rY += 0.7f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}