#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

struct Vec3 { float x, y, z; };
struct Vec2 { float u, v; };
struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct PengerMesh {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
};

PengerMesh loadPenger(const std::string& path) {
    PengerMesh mesh;
    std::ifstream file(path);
    if (!file) { std::cerr << "Fehler: " << path << " nicht gefunden\n"; return mesh; }

    std::vector<Vec3> positions;
    std::vector<Vec2> uvs;
    std::vector<std::vector<int>> faces;
    std::string line;
    int section = 0;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line.find("---") == 0) { section++; continue; }

        std::istringstream iss(line);
        if (section == 0) {
            Vec3 v; iss >> v.x >> v.y >> v.z;
            positions.push_back(v);
        }
        else if (section == 1) {
            // Edges – überspringen
        }
        else if (section == 2) {
            std::vector<int> face;
            int idx;
            while (iss >> idx) face.push_back(idx);
            if (face.size() >= 3) faces.push_back(face);
        }
        else if (section == 3) {
            Vec2 uv; iss >> uv.u >> uv.v;
            uvs.push_back(uv);
        }
    }
    file.close();

    if (positions.empty()) return mesh;

    // Normale pro Face berechnen
    std::vector<Vec3> normals(positions.size(), {0,0,0});
    std::vector<int> normalCount(positions.size(), 0);

    for (auto& f : faces) {
        Vec3 v0 = positions[f[0]], v1 = positions[f[1]], v2 = positions[f[2]];
        Vec3 e1 = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
        Vec3 e2 = {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
        Vec3 n = {
            e1.y * e2.z - e1.z * e2.y,
            e1.z * e2.x - e1.x * e2.z,
            e1.x * e2.y - e1.y * e2.x
        };
        float len = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
        if (len > 0.001f) { n.x /= len; n.y /= len; n.z /= len; }

        for (int idx : f) {
            normals[idx].x += n.x; normals[idx].y += n.y; normals[idx].z += n.z;
            normalCount[idx]++;
        }
    }
    for (size_t i = 0; i < normals.size(); i++) {
        if (normalCount[i] > 0) {
            normals[i].x /= normalCount[i];
            normals[i].y /= normalCount[i];
            normals[i].z /= normalCount[i];
            float len = std::sqrt(normals[i].x*normals[i].x + normals[i].y*normals[i].y + normals[i].z*normals[i].z);
            if (len > 0.001f) { normals[i].x /= len; normals[i].y /= len; normals[i].z /= len; }
        }
    }

    bool hasUV = (uvs.size() >= positions.size());
    for (size_t i = 0; i < positions.size(); i++) {
        Vec2 uv = hasUV ? uvs[i] : Vec2{0,0};
        mesh.verts.push_back({positions[i].x, positions[i].y, positions[i].z,
                              normals[i].x, normals[i].y, normals[i].z, uv.u, uv.v});
    }
    for (auto& f : faces) {
        for (int i = 1; i < (int)f.size() - 1; i++) {
            mesh.indices.push_back(f[0]);
            mesh.indices.push_back(f[i]);
            mesh.indices.push_back(f[i+1]);
        }
    }
    return mesh;
}

GLuint makeTexture() {
    const int W = 256, H = 256;
    std::vector<unsigned char> pixels(W * H * 3);
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            int i = (y * W + x) * 3;
            bool chk = ((x/32)%2 == 0) ^ ((y/32)%2 == 0);
            if (chk) { pixels[i]=220; pixels[i+1]=160; pixels[i+2]=60; }
            else     { pixels[i]=60;  pixels[i+1]=60;  pixels[i+2]=80; }
        }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    return tex;
}

int main() {
    std::string filePath = "wuerfel.penger";
    PengerMesh mesh = loadPenger(filePath);
    if (mesh.verts.empty()) {
        std::cerr << "Keine Daten in " << filePath << "\n";
        return -1;
    }

    if (!glfwInit()) return -1;
    GLFWwindow* win = glfwCreateWindow(900, 700, "Projekt 52 - Penger Viewer", NULL, NULL);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);

    GLfloat lightAmbient[] = {0.2f, 0.2f, 0.3f, 1.0f};
    GLfloat lightDiffuse[] = {0.9f, 0.9f, 1.0f, 1.0f};
    GLfloat lightSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    GLuint tex = makeTexture();

    GLuint vbo, ibo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.verts.size() * sizeof(Vertex), mesh.verts.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = 900.0f/700.0f;
    float fov = 0.05f;
    glFrustum(-fov*aspect, fov*aspect, -fov, fov, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    std::cout << mesh.verts.size() << " Vertices, " << mesh.indices.size() << " Indices geladen\n";

    while (!glfwWindowShouldClose(win)) {
        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;

        float t = glfwGetTime();
        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        glTranslatef(0.0f, 0.0f, -4.0f);
        glRotatef(t * 30.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(15.0f + std::sin(t * 0.3f) * 10.0f, 1.0f, 0.0f, 0.0f);

        GLfloat lightPos[] = {5.0f, 8.0f, 5.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

        glColor3f(1, 1, 1);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (void*)0);
        glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)(sizeof(float)*3));
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)(sizeof(float)*6));

        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        // Gitter
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.2f, 0.2f, 0.3f);
        glBegin(GL_LINES);
        for (int i = -10; i <= 10; i++) {
            glVertex3f(i, -1.5f, -10); glVertex3f(i, -1.5f, 10);
            glVertex3f(-10, -1.5f, i); glVertex3f(10, -1.5f, i);
        }
        glEnd();
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
