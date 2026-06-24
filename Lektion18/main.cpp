#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>

float rX = 0.0f, rY = 0.0f;
int renderMode = 0; // 0 = Kugel, 1 = Zylinder/Kegel

// Zeichnet eine mathematische Kugel (Sphere) über Längen- und Breitengrade
void drawSphere(float radius, int lingen, int breiten) {
    for (int i = 0; i <= lingen; ++i) {
        float lat0 = 3.14159265f * (-0.5f + (float)(i - 1) / lingen);
        float z0  = std::sin(lat0);
        float zr0 = std::cos(lat0);

        float lat1 = 3.14159265f * (-0.5f + (float)i / lingen);
        float z1  = std::sin(lat1);
        float zr1 = std::cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= breiten; ++j) {
            float lng = 2.0f * 3.14159265f * (float)(j - 1) / breiten;
            float x = std::cos(lng);
            float y = std::sin(lng);

            glNormal3f(x * zr0, y * zr0, z0); glVertex3f(radius * x * zr0, radius * y * zr0, radius * z0);
            glNormal3f(x * zr1, y * zr1, z1); glVertex3f(radius * x * zr1, radius * y * zr1, radius * z1);
        }
        glEnd();
    }
}

// Zeichnet einen mathematischen Zylinder / Kegel
void drawCylinder(float baseRadius, float topRadius, float height, int slices) {
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float angle = 2.0f * 3.14159265f * float(i) / float(slices);
        float x = std::cos(angle);
        float y = std::sin(angle);

        glNormal3f(x, y, 0.0f);
        glVertex3f(baseRadius * x, baseRadius * y, 0.0f);
        glVertex3f(topRadius * x,  topRadius * y,  height);
    }
    glEnd();
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 18 - Quadrics (Formen)", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL);
    GLfloat lp[]={5.0f,5.0f,5.0f,1.0f}; glLightfv(GL_LIGHT0, GL_POSITION, lp);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    double lastTime = glfwGetTime();

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        // Modus umschalten mit der LEERTASTE
        if(glfwGetKey(w, GLFW_KEY_SPACE)==GLFW_PRESS && glfwGetTime() - lastTime > 0.3) {
            renderMode = (renderMode + 1) % 2;
            lastTime = glfwGetTime();
        }

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -6.0f);
        glRotatef(rX, 1.0f, 0.0f, 0.0f); glRotatef(rY, 0.0f, 1.0f, 0.0f);

        glColor3f(0.0f, 0.8f, 1.0f);
        if(renderMode == 0) {
            drawSphere(1.5f, 24, 24);
        } else {
            // Ein TopRadius von 0.0f macht aus dem Zylinder automatisch einen Kegel!
            glTranslatef(0.0f, 0.0f, -1.0f);
            drawCylinder(1.2f, 0.0f, 2.0f, 24);
        }

        rX += 0.3f; rY += 0.5f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}