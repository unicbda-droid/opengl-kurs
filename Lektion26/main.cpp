#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>

float rX = 0.0f, rY = 0.0f;

void drawCube() {
    glBegin(GL_QUADS);
        // Vorderseite
        glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
        // Rückseite
        glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f);
        // Oberseite
        glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
        // Unterseite
        glColor3f(1.0f, 1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glEnd();
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 26 - Echte Spiegelungen & Clipping", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    // Definition der Clipping-Ebene (Gleichung: 0x + 1y + 0z + 0d = 0 -> Schneidet exakt horizontal bei Y=0)
    // Alles mit negativem Y (unter dem Boden) wird durchgelassen, alles darueber abgeschnitten
    double eqplane[] = {0.0, -1.0, 0.0, 0.0};

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -4.0f);

        // 1. SCHRITT: Den gespiegelten Würfel UNTER dem Boden zeichnen
        glPushMatrix();
            // Clipping aktivieren und Ebene setzen
            glEnable(GL_CLIP_PLANE0);
            glClipPlane(GL_CLIP_PLANE0, eqplane);
            
            // Welt spiegeln (Y-Achse invertieren)
            glScalef(1.0f, -1.0f, 1.0f);
            
            // Wuerfel-Transformationen (leicht nach oben versetzt)
            glTranslatef(0.0f, 0.8f, 0.0f);
            glRotatef(rX, 1.0f, 0.0f, 0.0f); glRotatef(rY, 0.0f, 1.0f, 0.0f);
            drawCube();
            
            glDisable(GL_CLIP_PLANE0);
        glPopMatrix();

        // 2. SCHRITT: Den transparenten Boden zeichnen (damit man die Spiegelung durchsieht)
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
            glColor4f(0.5f, 0.5f, 0.5f, 0.4f); // Halbtransparentes Grau
            glVertex3f(-2.0f, 0.0f, -2.0f);
            glVertex3f( 2.0f, 0.0f, -2.0f);
            glVertex3f( 2.0f, 0.0f,  2.0f);
            glVertex3f(-2.0f, 0.0f,  2.0f);
        glEnd();
        glDisable(GL_BLEND);

        // 3. SCHRITT: Den echten Würfel OBERHALB des Bodens zeichnen
        glPushMatrix();
            glTranslatef(0.0f, 0.8f, 0.0f);
            glRotatef(rX, 1.0f, 0.0f, 0.0f); glRotatef(rY, 0.0f, 1.0f, 0.0f);
            drawCube();
        glPopMatrix();

        rX += 0.4f; rY += 0.6f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}