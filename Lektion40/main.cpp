#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Definitionen fuer die Fog-Coord-Erweiterung
#ifndef GL_EXT_fog_coord
#define GL_FOG_COORDINATE_SOURCE_EXT         0x8470
#define GL_FOG_COORDINATE_EXT                0x8471
#define GL_FOG_COORDINATE_ARRAY_EXT          0x8472
typedef void (APIENTRY * PFNGLFOGCOORDPOINTEREXTPROC) (GLenum type, GLsizei stride, const void *pointer);
#endif

// Hier erstellen wir den Funktionszeiger, den wir manuell belegen
PFNGLFOGCOORDPOINTEREXTPROC glFogCoordPointer = nullptr;

float zOffset = -2.0f;
GLuint textureID;

GLfloat floorVertices[] = {
    -10.0f, -1.0f,   0.0f,
     10.0f, -1.0f,   0.0f,
     10.0f, -1.0f, -100.0f,
    -10.0f, -1.0f, -100.0f
};

GLfloat floorTexCoords[] = {
    0.0f,  0.0f,
    5.0f,  0.0f,
    5.0f, 25.0f,
    0.0f, 25.0f
};

GLfloat fogCoordinates[] = {
    0.0f, 
    0.0f, 
    1.0f, 
    1.0f  
};

void createTexture() {
    GLubyte textureData[64][64][3];
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            int c = ((((i & 0x8) == 0) ^ ((j & 0x8) == 0))) * 255;
            textureData[i][j][0] = (GLubyte)c;
            textureData[i][j][1] = (GLubyte)(c * 0.5f);
            textureData[i][j][2] = (GLubyte)0;
        }
    }
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 16x Anisotropie-ID
    glTexParameterf(GL_TEXTURE_2D, 0x84FE, 16.0f);
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 40 - Volumetrischer Schichten-Nebel", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w); 
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // Funktion zur Laufzeit aus dem NVIDIA-Treiber laden
    glFogCoordPointer = (PFNGLFOGCOORDPOINTEREXTPROC)glfwGetProcAddress("glFogCoordPointerEXT");
    if (!glFogCoordPointer) {
        std::cerr << "Fehler: Deine Grafikkarte unterstuetzt glFogCoordPointer nicht!" << std::endl;
        return -1;
    }

    createTexture();

    glEnable(GL_FOG);
    GLfloat fogColor[4] = {0.05f, 0.05f, 0.08f, 1.0f}; 
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);     
    glFogf(GL_FOG_START, 0.0f);         
    glFogf(GL_FOG_END, 1.0f);           

    glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_FOG_COORDINATE_ARRAY_EXT);

    glVertexPointer(3, GL_FLOAT, 0, floorVertices);
    glTexCoordPointer(2, GL_FLOAT, 0, floorTexCoords);
    glFogCoordPointer(GL_FLOAT, 0, fogCoordinates); 

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 150.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        if(glfwGetKey(w, GLFW_KEY_UP)==GLFW_PRESS) zOffset -= 0.2f;
        if(glfwGetKey(w, GLFW_KEY_DOWN)==GLFW_PRESS) zOffset += 0.2f;

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 
        
        glTranslatef(0.0f, 0.5f, zOffset);
        glRotatef(10.0f, 1.0f, 0.0f, 0.0f);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glDrawArrays(GL_QUADS, 0, 4);

        glfwSwapBuffers(w); glfwPollEvents();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_FOG_COORDINATE_ARRAY_EXT);
    glfwTerminate(); return 0;
}
