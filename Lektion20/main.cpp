#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <cmath>

GLuint texMask, texColor;
float roll = 0.0f;

// Generiert die prozeduralen Texturen (Maske und Farbe)
void createMaskTextures() {
    std::vector<GLubyte> maskImg(64*64*3);
    std::vector<GLubyte> colorImg(64*64*3);
    
    for(int i=0; i<64; i++) {
        for(int j=0; j<64; j++) {
            int idx = (i*64+j)*3;
            // Berechne einen Kreis in der Mitte
            float dx = float(j - 32); float dy = float(i - 32);
            bool inCircle = (dx*dx + dy*dy) < 400.0f; // Radius 20
            
            if(inCircle) {
                // Maske: Objekt ist SCHWARZ
                maskImg[idx]=0; maskImg[idx+1]=0; maskImg[idx+2]=0;
                // Farbe: Objekt ist BUNTER STREIFEN
                colorImg[idx]=255; colorImg[idx+1]=128; colorImg[idx+2]=0;
            } else {
                // Maske: Hintergrund ist WEISS
                maskImg[idx]=255; maskImg[idx+1]=255; maskImg[idx+2]=255;
                // Farbe: Hintergrund ist SCHWARZ
                colorImg[idx]=0; colorImg[idx+1]=0; colorImg[idx+2]=0;
            }
        }
    }
    // Masken-Textur laden
    glGenTextures(1, &texMask);
    glBindTexture(GL_TEXTURE_2D, texMask);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, maskImg.data());
    
    // Farb-Textur laden
    glGenTextures(1, &texColor);
    glBindTexture(GL_TEXTURE_2D, texColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, colorImg.data());
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 20 - Masking", NULL, NULL);
    glfwMakeContextCurrent(w);
    glEnable(GL_TEXTURE_2D); createMaskTextures();

    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        
        // Hintergrund mit einem farbigen Muster füllen, damit man die Transparenz sieht
        glClearColor(0.1f, 0.4f, 0.2f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef(roll, 0.0f, 0.0f, 1.0f);

        // SCHRITT 1: Maske zeichnen (Stenzt das Loch)
        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, texMask);
        glBlendFunc(GL_DST_COLOR, GL_ZERO); // Logisches UND-Gatter
        glEnable(GL_BLEND);
        
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f,0.0f);
            glTexCoord2f(1.0f,0.0f); glVertex3f( 1.0f,-1.0f,0.0f);
            glTexCoord2f(1.0f,1.0f); glVertex3f( 1.0f, 1.0f,0.0f);
            glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f, 1.0f,0.0f);
        glEnd();

        // SCHRITT 2: Farbe zeichnen (Füllt das Loch)
        glBindTexture(GL_TEXTURE_2D, texColor);
        glBlendFunc(GL_ONE, GL_ONE); // Logisches ODER-Gatter (Addition)
        
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f,0.0f);
            glTexCoord2f(1.0f,0.0f); glVertex3f( 1.0f,-1.0f,0.0f);
            glTexCoord2f(1.0f,1.0f); glVertex3f( 1.0f, 1.0f,0.0f);
            glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f, 1.0f,0.0f);
        glEnd();

        roll += 1.0f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}