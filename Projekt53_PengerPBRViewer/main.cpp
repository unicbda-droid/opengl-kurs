#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <cstring>

// ========== VEC3 ==========
struct Vec3 { float x, y, z; };
struct Vec2 { float u, v; };
struct Vertex {
    float x, y, z, nx, ny, nz, u, v;
};

// ========== PENGER v4 LOADER ==========
struct PengerMat {
    std::string diffuse, normal, bump, height;
};
struct PengerMesh {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
    PengerMat mat;
};

static PengerMesh loadPenger(const std::string& path) {
    PengerMesh mesh;
    std::ifstream f(path);
    if (!f) { std::cerr << "Fehler: " << path << " nicht gefunden\n"; return mesh; }

    std::vector<Vec3> pos;
    std::vector<Vec2> uv;
    std::vector<std::vector<int>> faces;
    int sec = 0;
    std::string line;

    auto trim = [](std::string& s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
    };

    while (std::getline(f, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line.find("---") == 0) { sec++; continue; }

        std::istringstream iss(line);
        if (sec == 0) {
            Vec3 v; iss >> v.x >> v.y >> v.z; pos.push_back(v);
        } else if (sec == 1) {
            // edges – skip
        } else if (sec == 2) {
            std::vector<int> face; int idx;
            while (iss >> idx) face.push_back(idx);
            if (face.size() >= 3) faces.push_back(face);
        } else if (sec == 3) {
            Vec2 t; iss >> t.u >> t.v; uv.push_back(t);
        } else if (sec == 4) {
            std::string key, val;
            iss >> key >> val;
            trim(key); trim(val);
            if (key == "diffuse") mesh.mat.diffuse = val;
            else if (key == "normal") mesh.mat.normal = val;
            else if (key == "bump") mesh.mat.bump = val;
            else if (key == "height") mesh.mat.height = val;
        }
    }
    f.close();

    if (pos.empty()) return mesh;

    // Normale per Face
    std::vector<Vec3> norms(pos.size(), {0,0,0});
    std::vector<int> ncnt(pos.size(), 0);
    for (auto& fc : faces) {
        Vec3 v0 = pos[fc[0]], v1 = pos[fc[1]], v2 = pos[fc[2]];
        Vec3 e1 = {v1.x-v0.x, v1.y-v0.y, v1.z-v0.z};
        Vec3 e2 = {v2.x-v0.x, v2.y-v0.y, v2.z-v0.z};
        Vec3 n = {e1.y*e2.z - e1.z*e2.y, e1.z*e2.x - e1.x*e2.z, e1.x*e2.y - e1.y*e2.x};
        float len = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
        if (len > 0.001f) { n.x/=len; n.y/=len; n.z/=len; }
        for (int idx : fc) { norms[idx].x+=n.x; norms[idx].y+=n.y; norms[idx].z+=n.z; ncnt[idx]++; }
    }
    for (size_t i=0; i<norms.size(); i++) if (ncnt[i]>0) {
        norms[i].x/=ncnt[i]; norms[i].y/=ncnt[i]; norms[i].z/=ncnt[i];
        float l = std::sqrt(norms[i].x*norms[i].x + norms[i].y*norms[i].y + norms[i].z*norms[i].z);
        if (l>0.001f) { norms[i].x/=l; norms[i].y/=l; norms[i].z/=l; }
    }

    bool hasUV = (uv.size() >= pos.size());
    for (size_t i=0; i<pos.size(); i++) {
        Vec2 t = hasUV ? uv[i] : Vec2{0,0};
        mesh.verts.push_back({pos[i].x, pos[i].y, pos[i].z, norms[i].x, norms[i].y, norms[i].z, t.u, t.v});
    }
    for (auto& fc : faces)
        for (size_t i=1; i+1<fc.size(); i++) {
            mesh.indices.push_back(fc[0]); mesh.indices.push_back(fc[i]); mesh.indices.push_back(fc[i+1]);
        }
    return mesh;
}

// ========== PROZEDURALE TEXTUREN ==========
static GLuint genTexture(int w, int h, bool normalMap, bool heightMap, int seed=0) {
    std::vector<unsigned char> pix(w * h * 3);
    for (int y=0; y<h; y++) for (int x=0; x<w; x++) {
        int i = (y*w + x)*3;
        float fx = (float)x/w, fy = (float)y/h;
        if (normalMap) {
            // Normal Map: konischer Farbverlauf
            float nx = (fx - 0.5f) * 2.0f;
            float ny = (fy - 0.5f) * 2.0f;
            float nz = std::sqrt(std::max(0.0f, 1.0f - nx*nx - ny*ny));
            pix[i]   = (unsigned char)((nx*0.5f+0.5f)*255);
            pix[i+1] = (unsigned char)((ny*0.5f+0.5f)*255);
            pix[i+2] = (unsigned char)(nz*255);
        } else if (heightMap) {
            // Height Map: welliges Gelände
            float hgt = std::sin(fx*12.0f + seed)*std::cos(fy*10.0f + seed*0.7f)*0.5f + 0.5f;
            hgt = hgt*hgt;
            unsigned char v = (unsigned char)(hgt * 255);
            pix[i]=v; pix[i+1]=v; pix[i+2]=v;
        } else {
            // Diffuse: Schachbrett mit Farben
            bool chk = (((int)(fx*8) + (int)(fy*8)) % 2) == ((x/16 + y/16) % 2 == 0);
            if (chk) { pix[i]=200; pix[i+1]=120+seed*30; pix[i+2]=60+seed*20; }
            else     { pix[i]=40+seed*10; pix[i+1]=40; pix[i+2]=60-seed*10; }
        }
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pix.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    return tex;
}

// ========== SHADER ==========
static GLuint loadShader(const std::string& path, GLenum type) {
    std::ifstream f(path);
    std::string src((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    const char* csrc = src.c_str();
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &csrc, nullptr);
    glCompileShader(sh);
    GLint ok; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]; GLint len;
        glGetShaderInfoLog(sh, 1024, &len, log);
        std::cerr << "Shader-Fehler (" << (type==GL_VERTEX_SHADER?"vert":"frag") << "): " << log << "\n";
    }
    return sh;
}

static GLuint buildProgram(const std::string& vertPath, const std::string& fragPath) {
    GLuint vs = loadShader(vertPath, GL_VERTEX_SHADER);
    GLuint fs = loadShader(fragPath, GL_FRAGMENT_SHADER);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) { char log[1024]; GLint len; glGetProgramInfoLog(prog, 1024, &len, log);
        std::cerr << "Link-Fehler: " << log << "\n"; }
    glDeleteShader(vs); glDeleteShader(fs);
    return prog;
}

// ========== CAMERA ==========
struct OrbitCam {
    float dist = 4.0f, angX = 0.3f, angY = 0.0f;
    int lastMX=0, lastMY=0;
    bool dragging=false;
};

// ========== MAIN ==========
int main() {
    std::string pengerFile = "kaefer.penger";
    auto mesh = loadPenger(pengerFile);
    if (mesh.verts.empty()) {
        std::cerr << "Lade " << pengerFile << " fehlgeschlagen\n";
        return -1;
    }

    if (!glfwInit()) return -1;
    GLFWwindow* win = glfwCreateWindow(1024, 768, "Projekt 53 - Penger PBR Viewer", NULL, NULL);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glewInit();

    glEnable(GL_DEPTH_TEST);

    // Texturen generieren
    GLuint texDiff = genTexture(256, 256, false, false, 0);
    GLuint texNorm = genTexture(256, 256, true,  false, 1);
    GLuint texBump = genTexture(256, 256, false, true,  2);
    GLuint texHgt  = genTexture(256, 256, false, true,  3);

    std::cout << "Prozedurale Texturen erzeugt (mit Mipmaps)\n";

    // Shader
    auto base = "C:\\BlenderProjekte\\Projekt Buch Germanische Bibel\\OpenGL_Kurs\\Projekt53_PengerPBRViewer";
    std::string vertPath = base + ("\\shader.vert");
    std::string fragPath = base + ("\\shader.frag");
    GLuint prog = buildProgram(vertPath, fragPath);
    if (!prog) { std::cerr << "Shader-Fehler\n"; return -1; }

    // VBO/IBO
    GLuint vbo, ibo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.verts.size() * sizeof(Vertex), mesh.verts.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    // Attribute
    GLint aPos = glGetAttribLocation(prog, "aPos");
    GLint aNorm = glGetAttribLocation(prog, "aNormal");
    GLint aUV = glGetAttribLocation(prog, "aUV");
    GLint uDiff = glGetUniformLocation(prog, "uDiffuse");
    GLint uNorm = glGetUniformLocation(prog, "uNormal");
    GLint uBump = glGetUniformLocation(prog, "uBump");
    GLint uHgt  = glGetUniformLocation(prog, "uHeight");
    GLint uLP   = glGetUniformLocation(prog, "uLightPos");
    GLint uLC   = glGetUniformLocation(prog, "uLightColor");
    GLint uAmb  = glGetUniformLocation(prog, "uAmbient");

    glUseProgram(prog);
    glUniform1i(uDiff, 0);
    glUniform1i(uNorm, 1);
    glUniform1i(uBump, 2);
    glUniform1i(uHgt,  3);

    OrbitCam cam;

    std::cout << "Projekt53: " << mesh.verts.size() << " Verts, " << mesh.indices.size() << " Indices geladen\n";
    std::cout << "Material: diffuse=[" << mesh.mat.diffuse << "] normal=[" << mesh.mat.normal
              << "] bump=[" << mesh.mat.bump << "] height=[" << mesh.mat.height << "]\n";
    std::cout << "(Texturen werden prozedural generiert)\n";
    std::cout << "Maus ziehen = Kamera orbiten, ESC = Ende\n";

    while (!glfwWindowShouldClose(win)) {
        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;

        // Camera orbit
        int mx, my;
        glfwGetCursorPos(win, &mx, &my);
        if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (!cam.dragging) { cam.lastMX=mx; cam.lastMY=my; cam.dragging=true; }
            cam.angY -= (mx - cam.lastMX) * 0.005f;
            cam.angX += (my - cam.lastMY) * 0.005f;
            cam.angX = std::max(-1.5f, std::min(1.5f, cam.angX));
            cam.lastMX=mx; cam.lastMY=my;
        } else cam.dragging = false;

        // Scroll zoom
        static double scroll=0;
        if (glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS) cam.dist -= 0.05f;
        if (glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS) cam.dist += 0.05f;
        cam.dist = std::max(1.5f, std::min(20.0f, cam.dist));

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Projektion + ModelView
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float a = 1024.0f/768.0f, f = 0.05f;
        glFrustum(-f*a, f*a, -f, f, 0.1f, 50.0f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float cx = cam.dist * std::cos(cam.angX) * std::sin(cam.angY);
        float cy = cam.dist * std::sin(cam.angX);
        float cz = cam.dist * std::cos(cam.angX) * std::cos(cam.angY);
        gluLookAt(cx, cy, cz, 0, 0, 0, 0, 1, 0);

        // Licht
        float t = glfwGetTime();
        Vec3 lPos = {std::sin(t*0.5f)*5.0f, 4.0f, std::cos(t*0.5f)*5.0f};
        glUniform3f(uLP, lPos.x, lPos.y, lPos.z);
        glUniform3f(uLC, 1.0f, 0.95f, 0.85f);
        glUniform3f(uAmb, 0.15f, 0.15f, 0.2f);

        // Texturen binden
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texDiff);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, texNorm);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, texBump);
        glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, texHgt);

        // Draw
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

        glEnableVertexAttribArray(aPos);
        glEnableVertexAttribArray(aNorm);
        glEnableVertexAttribArray(aUV);

        glVertexAttribPointer(aPos,  3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glVertexAttribPointer(aNorm, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float)*3));
        glVertexAttribPointer(aUV,   2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float)*6));

        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)0);

        glDisableVertexAttribArray(aPos);
        glDisableVertexAttribArray(aNorm);
        glDisableVertexAttribArray(aUV);

        // Gitter
        glUseProgram(0);
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_LINES);
        glColor3f(0.15f, 0.15f, 0.2f);
        for (int i=-10; i<=10; i++) {
            glVertex3f(i, -1.2f, -10); glVertex3f(i, -1.2f, 10);
            glVertex3f(-10, -1.2f, i); glVertex3f(10, -1.2f, i);
        }
        glEnd();

        // Lichtposition als Kugel
        glColor3f(1,1,0.5f);
        glPointSize(8);
        glBegin(GL_POINTS);
        glVertex3f(lPos.x, lPos.y, lPos.z);
        glEnd();

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
