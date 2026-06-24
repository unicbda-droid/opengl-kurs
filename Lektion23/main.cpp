#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <cmath>

GLuint texReflect;
float rX = 0.0f, rY = 0.0f;

// Generiert eine prozedurale "Umgebungs-Textur" (ein helles psychedelisches Kreuz/Raum)
void createReflectTexture() {
    std::vector<GLubyte> img(128*128*3);
    for(int i=0; i<128; i++) {
        for(int j=0; j<128; j++) {
            int idx = (i*128+j)*3;
            // Erzeugt ein weiches, kreisförmiges Glühen für die "Spiegelung"
            float dx = float(j - 64); float dy = float(i - 64);
            float dist = std::sqrt(dx*dx + dy*dy);
            GLubyte intensity = (dist < 64.0f) ? GLubyte((64.0f - dist) * 3.5f) : 0;
            
            img[idx] = intensity;          // Rot-Anteil
            img[idx+1] = intensity / 2;    // Grün-Anteil
            img[idx+2] = 255 - intensity;  // Blau-Anteil
        }
    }
    glGenTextures(1, &texReflect);
    glBindTexture(GL_TEXTURE_2D, texReflect);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
}

// Hilfsfunktion für eine Kugel aus Lektion 18
void drawSphere(float radius, int lingen, int breiten) {
    for (int i = 0; i <= lingen; ++i) {
        float lat0 = 3.14159265f * (-0.5f + (float)(i - 1) / lingen);
        float z0  = std::sin(lat0); float zr0 = std::cos(lat0);
        float lat1 = 3.14159265f * (-0.5f + (float)i / lingen);
        float z1  = std::sin(lat1); float zr1 = std::cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= breiten; ++j) {
            float lng = 2.0f * 3.14159265f * (float)(j - 1) / breiten;
            float x = std::cos(lng); float y = std::sin(lng);
            glNormal3f(x * zr0, y * zr0, z0); glVertex3f(radius * x * zr0, radius * y * zr0, radius * z0);
            glNormal3f(x * zr1, y * zr1, z1); glVertex3f(radius * x * zr1, radius * y * zr1, radius * z1);
        }
        glEnd();
    }
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 23 - Sphere Mapping", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST); createReflectTexture();

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    // Hier aktivieren wir die automatische Textur-Koordinaten-Generierung
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texReflect);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -4.5f);
        glRotatef(rX, 1.0f, 0.0f, 0.0f); glRotatef(rY, 0.0f, 1.0f, 0.0f);

        // Automatische Generierung für die Achsen S und T (entspricht U und V) einschalten
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawSphere(1.4f, 32, 32);

        // Wieder ausschalten, damit andere Objekte normal gezeichnet werden könnten
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);

        rX += 0.4f; rY += 0.6f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}