#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

float rY = 0.0f;
float zOffset = -4.0f; // Wir machen die Kamera beweglich, um den Effekt zu sehen!

GLfloat cubeVertices[] = {
    -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f
};

GLfloat texCoords[] = {
    0.0f,0.0f,  1.0f,0.0f,  1.0f,1.0f,  0.0f,1.0f,
    1.0f,0.0f,  1.0f,1.0f,  0.0f,1.0f,  0.0f,0.0f,
    0.0f,1.0f,  0.0f,0.0f,  1.0f,0.0f,  1.0f,1.0f,
    1.0f,1.0f,  0.0f,1.0f,  0.0f,0.0f,  1.0f,0.0f,
    1.0f,0.0f,  1.0f,1.0f,  0.0f,1.0f,  0.0f,0.0f,
    0.0f,0.0f,  1.0f,0.0f,  1.0f,1.0f,  0.0f,1.0f
};

GLuint textureID;

void createMipmappedTexture() {
    GLubyte textureData[64][64][3];
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            int c = ((((i & 0x4) == 0) ^ ((j & 0x4) == 0))) * 255; // Etwas feineres Muster
            textureData[i][j][0] = (GLubyte)c;
            textureData[i][j][1] = (GLubyte)c;
            textureData[i][j][2] = (GLubyte)c;
        }
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Mipmap-Kette selbst generieren (ersetzt gluBuild2DMipmaps)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Mipmap-Filter aktivieren: Lineare Interpolation AUCH zwischen den Mipmap-Stufen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 35 - Mipmapping", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    createMipmappedTexture();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, cubeVertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    std::cout << "Nutze Pfeiltaste HOCH / RUNTER, um den Wuerfel in die Ferne zu jagen!" << std::endl;

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        // Kamera-Steuerung
        if(glfwGetKey(w, GLFW_KEY_UP)==GLFW_PRESS) zOffset -= 0.1f;
        if(glfwGetKey(w, GLFW_KEY_DOWN)==GLFW_PRESS) zOffset += 0.1f;

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, zOffset);
        glRotatef(rY, 1.0f, 1.0f, 0.0f);

        glDrawArrays(GL_QUADS, 0, 24);

        rY += 0.3f;
        glfwSwapBuffers(w); glfwPollEvents();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glfwTerminate(); return 0;
}
