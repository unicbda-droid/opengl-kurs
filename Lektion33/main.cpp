#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>

float rY = 0.0f;

// 1. Die 3D-Eckpunkte des Würfels
GLfloat cubeVertices[] = {
    -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f
};

// 2. Das NEUE Array: Wo treffen die 3D-Punkte auf das 2D-Bild? (U/V oder S/T Koordinaten)
GLfloat texCoords[] = {
    0.0f,0.0f,  1.0f,0.0f,  1.0f,1.0f,  0.0f,1.0f,
    1.0f,0.0f,  1.0f,1.0f,  0.0f,1.0f,  0.0f,0.0f,
    0.0f,1.0f,  0.0f,0.0f,  1.0f,0.0f,  1.0f,1.0f,
    1.0f,1.0f,  0.0f,1.0f,  0.0f,0.0f,  1.0f,0.0f,
    1.0f,0.0f,  1.0f,1.0f,  0.0f,1.0f,  0.0f,0.0f,
    0.0f,0.0f,  1.0f,0.0f,  1.0f,1.0f,  0.0f,1.0f
};

GLuint textureID;

void createProceduralTexture() {
    // Da wir keine Bilddatei mitschleppen wollen, generieren wir ein schickes Schachbrettmuster im RAM
    GLubyte textureData[64][64][3];
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            int c = ((((i & 0x8) == 0) ^ ((j & 0x8) == 0))) * 255;
            textureData[i][j][0] = (GLubyte)c;       // Rot
            textureData[i][j][1] = (GLubyte)(c / 2); // Grün (gibt ein schickes Orange/Braun)
            textureData[i][j][2] = (GLubyte)(255 - c); // Blau
        }
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 33 - Texture Arrays", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w); 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    createProceduralTexture();

    // HIER AKTIVIEREN WIR DIE BLÖCKE: Geometrie und Textur-Zuweisung
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, cubeVertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -4.0f);
        glRotatef(rY, 1.0f, 1.0f, 0.3f);

        // Ein einziger Befehl zeichnet den komplett texturierten Würfel!
        glDrawArrays(GL_QUADS, 0, 24);

        rY += 0.5f;
        glfwSwapBuffers(w); glfwPollEvents();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glfwTerminate(); return 0;
}
