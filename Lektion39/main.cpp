#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Definitionen fuer die Anisotropie-Erweiterung (falls nicht im Header vorhanden)
#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT        0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT    0x84FF
#endif

float rY = 0.0f;
bool enableAnisotropy = true;
GLfloat maxAnisotropy = 1.0f;
GLuint textureID;

// Riesige, flache Ebene (wie eine Landebahn oder Straße)
GLfloat floorVertices[] = {
    -10.0f, -1.0f,  0.0f,
     10.0f, -1.0f,  0.0f,
     10.0f, -1.0f, -80.0f,
    -10.0f, -1.0f, -80.0f
};

// Wir kacheln die Textur massiv (20-mal in die Tiefe), damit man den Effekt extrem sieht
GLfloat floorTexCoords[] = {
    0.0f,  0.0f,
    5.0f,  0.0f,
    5.0f, 20.0f,
    0.0f, 20.0f
};

void createFineTexture() {
    // Ein sehr feines Rastermuster, das ohne anisotropen Filter sofort im Matsch versinkt
    GLubyte textureData[128][128][3];
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            // Feines Linienmuster erzeugen
            int c = (((i % 8 < 2) || (j % 8 < 2))) ? 255 : 50;
            textureData[i][j][0] = (GLubyte)c; // Cyan-Farbton
            textureData[i][j][1] = (GLubyte)(c * 0.8f);
            textureData[i][j][2] = (GLubyte)255;
        }
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Mipmaps generieren (Grundvoraussetzung!)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Normaler trilinearer Filter als Basis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 39 - 16x Anisotrope Filterung", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    createFineTexture();

    // Abfragen, wie viel Anisotropie deine Grafikkarte maximal kann
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    std::cout << "Deine Grafikkarte unterstuetzt maximal: " << maxAnisotropy << "x Anisotropie!" << std::endl;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, floorVertices);
    glTexCoordPointer(2, GL_FLOAT, 0, floorTexCoords);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 150.0f);
    glMatrixMode(GL_MODELVIEW);

    std::cout << "Druecke DIE LEERTASTE, um Anisotrope Filterung AN/AUS zu schalten!" << std::endl;

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        // Umschalter
        static bool spacePressed = false;
        if(glfwGetKey(w, GLFW_KEY_SPACE)==GLFW_PRESS && !spacePressed) {
            enableAnisotropy = !enableAnisotropy;
            spacePressed = true;
            std::cout << "Anisotrope Filterung: " << (enableAnisotropy ? "AN (16x)" : "AUS (Trilinearer Matsch)") << std::endl;
        }
        if(glfwGetKey(w, GLFW_KEY_SPACE)==GLFW_RELEASE) spacePressed = false;

        glClearColor(0.01f, 0.01f, 0.02f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 
        
        // Kamera leicht erhoeht positionieren und flach ueber die Ebene schauen lassen
        glTranslatef(0.0f, 0.5f, -2.0f);
        glRotatef(15.0f, 1.0f, 0.0f, 0.0f); // Leicht nach unten nicken

        // Filter im laufenden Betrieb umschalten
        glBindTexture(GL_TEXTURE_2D, textureID);
        if(enableAnisotropy) {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        } else {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f); // 1.0f bedeutet AUS
        }

        glDrawArrays(GL_QUADS, 0, 4);

        glfwSwapBuffers(w); glfwPollEvents();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glfwTerminate(); return 0;
}
