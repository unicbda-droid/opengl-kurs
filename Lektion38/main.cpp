#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <vector>
#include <fstream>

GLuint textureID;
float xrot = 0, yrot = 0;

struct BMPHeader {
    char id[2]; int fileSize; short reserved[2]; int dataOffset;
    int headerSize; int width; int height; short planes; short bpp;
    int compression; int imageSize; int xPPM; int yPPM; int colorsUsed; int colorsImportant;
};

bool loadBMP(const char* path, unsigned char*& data, int& w, int& h) {
    std::ifstream f(path, std::ios::binary);
    if(!f) return false;
    BMPHeader hdr;
    f.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    if(hdr.id[0]!='B'||hdr.id[1]!='M'||hdr.bpp!=24) return false;
    w=hdr.width; h=std::abs(hdr.height);
    int pad = (4-(w*3%4))%4;
    data = new unsigned char[(w*3+pad)*h];
    f.read(reinterpret_cast<char*>(data), (w*3+pad)*h);
    f.close();
    return true;
}

void createProceduralTexture() {
    int w=64, h=64; std::vector<GLubyte> img(w*h*3);
    for(int i=0;i<h;i++) for(int j=0;j<w;j++) {
        GLubyte c=((((i&0x8)==0)^((j&0x8)==0)))*200+55;
        int idx=(i*w+j)*3; img[idx]=c; img[idx+1]=c/2; img[idx+2]=c/4;
    }
    glGenTextures(1,&textureID); glBindTexture(GL_TEXTURE_2D,textureID);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0,GL_RGB,GL_UNSIGNED_BYTE,img.data());
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800,600,"NeHe Lektion 38 - Textur aus .BMP",NULL,NULL);
    if(!w){glfwTerminate();return -1;}
    glfwMakeContextCurrent(w); glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST); glEnable(GL_TEXTURE_2D);
    createProceduralTexture();
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    float a=800.f/600.f, fFOV=45.f*3.14159f/180.f, fH=tan(fFOV/2.f)*0.1f, fW=fH*a;
    glFrustum(-fW,fW,-fH,fH,0.1f,100.f);
    glMatrixMode(GL_MODELVIEW);
    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w,GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(w,true);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); glLoadIdentity();
        glTranslatef(0,0,-5); glRotatef(xrot,1,0,0); glRotatef(yrot,0,1,0);
        glBindTexture(GL_TEXTURE_2D,textureID);
        glBegin(GL_QUADS);
            glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
            glTexCoord2f(1,0); glVertex3f( 1,-1, 1);
            glTexCoord2f(1,1); glVertex3f( 1, 1, 1);
            glTexCoord2f(0,1); glVertex3f(-1, 1, 1);
        glEnd();
        xrot+=0.3f; yrot+=0.2f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}
