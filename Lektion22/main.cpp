#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <cmath>

GLuint texBase, texBump;
float rX = 0.0f, rY = 0.0f;

// Generiert eine normale Textur und eine Relief-Bumpmap
void createBumpTextures() {
    std::vector<GLubyte> baseImg(64*64*3);
    std::vector<GLubyte> bumpImg(64*64*3);
    
    for(int i=0; i<64; i++) {
        for(int j=0; j<64; j++) {
            int idx = (i*64+j)*3;
            // Ein einfaches Streifenmuster für die Basistextur
            GLubyte color = ((i / 8) % 2 == 0) ? 200 : 50;
            baseImg[idx] = color; baseImg[idx+1] = color / 2; baseImg[idx+2] = 50;
            
            // Die Bumpmap enthält die Kanten (Graustufen für Höhe)
            GLubyte bump = (i % 8 == 0 || j % 8 == 0) ? 255 : 128;
            bumpImg[idx] = bump; bumpImg[idx+1] = bump; bumpImg[idx+2] = bump;
        }
    }
    glGenTextures(1, &texBase);
    glBindTexture(GL_TEXTURE_2D, texBase);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, baseImg.data());
    
    glGenTextures(1, &texBump);
    glBindTexture(GL_TEXTURE_2D, texBump);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, bumpImg.data());
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 22 - Bump Mapping", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST); createBumpTextures();

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -4.0f);
        glRotatef(rX, 1.0f, 0.0f, 0.0f); glRotatef(rY, 0.0f, 1.0f, 0.0f);

        // PASS 1: Bumpmap mit normaler Helligkeit zeichnen
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texBump);
        glDisable(GL_BLEND);
        
        glBegin(GL_QUADS);
            glNormal3f(0.0f, 0.0f, 1.0f);
            glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f,0.0f);
            glTexCoord2f(1.0f,0.0f); glVertex3f( 1.0f,-1.0f,0.0f);
            glTexCoord2f(1.0f,1.0f); glVertex3f( 1.0f, 1.0f,0.0f);
            glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f, 1.0f,0.0f);
        glEnd();

        // PASS 2: Invertierte Bumpmap leicht versetzt drüberblenden (Invertiertes Embossing)
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE); // Additive Lichtberechnung
        
        // Wir simulieren eine Lichtverschiebung über minimale Textur-Offsets
        float offset = 0.01f * std::sin(rX * 0.05f);
        
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f + offset, 0.0f + offset); glVertex3f(-1.0f,-1.0f,0.0f);
            glTexCoord2f(1.0f + offset, 0.0f + offset); glVertex3f( 1.0f,-1.0f,0.0f);
            glTexCoord2f(1.0f + offset, 1.0f + offset); glVertex3f( 1.0f, 1.0f,0.0f);
            glTexCoord2f(0.0f + offset, 1.0f + offset); glVertex3f(-1.0f, 1.0f,0.0f);
        glEnd();

        // PASS 3: Albedo / Basistextur drübermultiplizieren
        glBindTexture(GL_TEXTURE_2D, texBase);
        glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f,0.0f);
            glTexCoord2f(1.0f,0.0f); glVertex3f( 1.0f,-1.0f,0.0f);
            glTexCoord2f(1.0f,1.0f); glVertex3f( 1.0f, 1.0f,0.0f);
            glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f, 1.0f,0.0f);
        glEnd();

        rX += 0.5f; rY += 0.8f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}