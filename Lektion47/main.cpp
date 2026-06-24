#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <vector>

float camX = 0.0f, camY = 15.0f, camZ = 40.0f;
float pitch = 25.0f, yaw = 0.0f;
const int GRID_SIZE = 100;

struct Vertex3D {
    float x, y, z;
    float nx, ny, nz; 
    float u, v;
};

GLuint vboID = 0;
GLuint textureID = 0;
int totalVertices = 0;

// Prozedurale Textur generieren (Ein feines Rausch-Muster)
void createProceduralTexture() {
    const int texWidth = 64;
    const int texHeight = 64;
    GLubyte texData[texWidth * texHeight * 3];

    for(int y = 0; y < texHeight; y++) {
        for(int x = 0; x < texWidth; x++) {
            // Ein mathematisches Streifen- und Rauschmuster kombinieren
            GLubyte r = (GLubyte)((std::sin(x * 0.5f) * 0.5f + 0.5f) * 120 + 100);
            GLubyte g = (GLubyte)((std::cos(y * 0.5f) * 0.5f + 0.5f) * 150 + 80);
            GLubyte b = (GLubyte)((x ^ y) % 255); // Digitales Rauschen ueber XOR-Logik

            int index = (y * texWidth + x) * 3;
            texData[index]     = r;
            texData[index + 1] = g;
            texData[index + 2] = b;
        }
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Mipmapping und Filterung fuer scharfe Texturen ohne Flimmern in der Ferne
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Texturdaten an die Grafikkarte senden und Mipmaps automatisch bauen lassen
    // (Aus Kompatibilitaetsgruenden nutzen wir hier GL_NEAREST Mipmap Generierung via glu-Ersatz manuell oder Standard-OpenGL-1.4-Logik)
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
}

void createTexturedTerrain() {
    std::vector<Vertex3D> vertices;

    for(int z = 0; z < GRID_SIZE - 1; z++) {
        for(int x = 0; x < GRID_SIZE; x++) {
            for(int row = 0; row < 2; row++) {
                int currentZ = z + row;
                float xPos = (float)x - (GRID_SIZE / 2.0f);
                float zPos = (float)currentZ - (GRID_SIZE / 2.0f);
                
                float yVal = std::sin(xPos * 0.15f) * std::cos(zPos * 0.15f) * 4.0f;
                yVal += std::sin(xPos * 0.5f) * 0.8f; 

                // Normalen berechnen
                float dx = 0.15f * std::cos(xPos * 0.15f) * std::cos(zPos * 0.15f) + 0.5f * std::cos(xPos * 0.5f);
                float dz = -0.15f * std::sin(xPos * 0.15f) * std::sin(zPos * 0.15f);
                float nx = -dx, ny = 1.0f, nz = -dz;
                float length = std::sqrt(nx*nx + ny*ny + nz*nz);
                nx /= length; ny /= length; nz /= length;

                // UV-Koordinaten skalieren (Die Textur wiederholt sich alle 5 Einheiten)
                float u = xPos * 0.2f;
                float v = zPos * 0.2f;

                vertices.push_back({xPos, yVal, zPos, nx, ny, nz, u, v});
            }
        }
    }

    totalVertices = vertices.size();

    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex3D), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void moveCamera(float speed, float angleOffset) {
    float rad = (yaw + angleOffset) * M_PI / 180.0f;
    camX -= std::sin(rad) * speed;
    camZ -= std::cos(rad) * speed;
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 47 - VBO Terrain mit Textur-Mapping", NULL, NULL);
    if(!w) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(w);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    createProceduralTexture();
    createTexturedTerrain();

    glEnable(GL_LIGHTING); 
    glEnable(GL_LIGHT0);   
    glEnable(GL_TEXTURE_2D); // Texturen global aktivieren

    GLfloat lightAmbient[]  = { 0.2f, 0.2f, 0.25f, 1.0f }; 
    GLfloat lightDiffuse[]  = { 1.0f, 1.0f, 0.95f, 1.0f }; 
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 200.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;

        if(glfwGetKey(w, GLFW_KEY_UP)==GLFW_PRESS)    pitch += 1.5f;
        if(glfwGetKey(w, GLFW_KEY_DOWN)==GLFW_PRESS)  pitch -= 1.5f;
        if(glfwGetKey(w, GLFW_KEY_LEFT)==GLFW_PRESS)  yaw += 1.5f;
        if(glfwGetKey(w, GLFW_KEY_RIGHT)==GLFW_PRESS) yaw -= 1.5f;
        if(glfwGetKey(w, GLFW_KEY_W)==GLFW_PRESS) moveCamera(0.3f, 0.0f);
        if(glfwGetKey(w, GLFW_KEY_S)==GLFW_PRESS) moveCamera(0.3f, 180.0f);

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); 

        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glTranslatef(-camX, -camY, -camZ);

        GLfloat lightPosition[] = { 30.0f, 40.0f, 20.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

        // Bei Texturen setzen wir die Modulations-Farbe auf Weiss, damit die Textur original faerbt
        glColor3f(1.0f, 1.0f, 1.0f);

        // VBO-BUFFER AKTIVIEREN
        glBindBuffer(GL_ARRAY_BUFFER, vboID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY); 
        glEnableClientState(GL_TEXTURE_COORD_ARRAY); // JETZT NEU: Textur-Pointer-Staat aktivieren

        // Berechnung der Byte-Offsets (Wo liegen die Daten im Struct?)
        // Position: ab Byte 0
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), (void*)0);
        // Normalen: nach 3 Floats (Position) -> 3 * 4 Bytes = 12 Bytes Offset
        glNormalPointer(GL_FLOAT, sizeof(Vertex3D), (void*)(sizeof(float) * 3)); 
        // UVs: nach 6 Floats (Pos + Normalen) -> 6 * 4 Bytes = 24 Bytes Offset
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3D), (void*)(sizeof(float) * 6));

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        for(int i = 0; i < GRID_SIZE - 1; i++) {
            glDrawArrays(GL_TRIANGLE_STRIP, i * (GRID_SIZE * 2), GRID_SIZE * 2);
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glfwSwapBuffers(w); glfwPollEvents();
    }

    glfwTerminate(); return 0;
}
