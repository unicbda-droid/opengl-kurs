#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

float rY = 0.0f;
bool enableAA = true; // Schalter fuer das Anti-Aliasing

// Die Eckpunkte fuer ein komplexeres Gitter-Objekt (Schnittpunkte einer Pyramide)
GLfloat tVertices[] = {
    -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   0.0f,  1.0f,  0.0f,
     1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,   0.0f,  1.0f,  0.0f,
     1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   0.0f,  1.0f,  0.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   0.0f,  1.0f,  0.0f
};

int main() {
    if(!glfwInit()) return -1;

    // HIER BESTELLEN WIR BEI DER GRAFIKKARTE MSAA (4 Samples fuer knackige Kanten)
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 37 - Gestochen scharfes Anti-Aliasing", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w);
    glewInit();
    glEnable(GL_DEPTH_TEST);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, tVertices);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    std::cout << "Druecke DIE LEERTASTE, um das Anti-Aliasing AN/AUS zu schalten!" << std::endl;

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        // Interaktiver Umschalter
        static bool spacePressed = false;
        if(glfwGetKey(w, GLFW_KEY_SPACE)==GLFW_PRESS && !spacePressed) {
            enableAA = !enableAA;
            spacePressed = true;
            std::cout << "Anti-Aliasing ist jetzt: " << (enableAA ? "AN (4x MSAA)" : "AUS (Treppchenbildung)") << std::endl;
        }
        if(glfwGetKey(w, GLFW_KEY_SPACE)==GLFW_RELEASE) spacePressed = false;

        glClearColor(0.02f, 0.02f, 0.03f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, -0.3f, -3.5f);
        glRotatef(rY, 0.0f, 1.0f, 0.0f);

        if(enableAA) {
            // Glättungs-Pipelines der Hardware aktivieren
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); // Maximale Qualitaet fordern
        } else {
            // Raw und ungezähmt (Pixelmatsch und Treppen)
            glDisable(GL_MULTISAMPLE);
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_BLEND);
        }

        // Wir zeichnen das Objekt als reines, gestochen scharfes Liniengitter
        glColor3f(0.0f, 1.0f, 0.7f); // Schickes Sci-Fi Cyan
        glLineWidth(2.0f);           // Linien etwas dicker, damit man das AA sieht
        glDrawArrays(GL_LINES, 0, 12);

        rY += 0.4f;
        glfwSwapBuffers(w); glfwPollEvents();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glfwTerminate(); return 0;
}
