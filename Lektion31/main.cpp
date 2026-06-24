#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

struct Vertex { float x, y, z; };
struct Face { int v1, v2, v3; };

std::vector<Vertex> vertices;
std::vector<Face> faces;
float rY = 0.0f;

const std::string objData = R"(
v 0.0 1.0 0.0
v -1.0 -1.0 1.0
v 1.0 -1.0 1.0
v 1.0 -1.0 -1.0
v -1.0 -1.0 -1.0
f 1 2 3
f 1 3 4
f 1 4 5
f 1 5 2
f 2 4 3
f 2 5 4
)";

void parseOBJ() {
    std::stringstream ss(objData);
    std::string line;
    while (std::getline(ss, line)) {
        if(line.empty()) continue;
        std::stringstream lineSS(line);
        std::string type;
        lineSS >> type;
        if (type == "v") {
            Vertex v;
            lineSS >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        } else if (type == "f") {
            Face f;
            lineSS >> f.v1 >> f.v2 >> f.v3;
            f.v1--; f.v2--; f.v3--;
            faces.push_back(f);
        }
    }
    std::cout << "Modell geladen: " << vertices.size() << " Vertices, " << faces.size() << " Faces." << std::endl;
}

int main() {
    if(!glfwInit()) return -1;
    GLFWwindow* w = glfwCreateWindow(800, 600, "Lektion 31 - Model Loading (.OBJ)", NULL, NULL);
    glfwMakeContextCurrent(w); glEnable(GL_DEPTH_TEST);
    parseOBJ();
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-0.0414f, 0.0414f, -0.031f, 0.031f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    while(!glfwWindowShouldClose(w)) {
        if(glfwGetKey(w, GLFW_KEY_ESCAPE)==GLFW_PRESS) break;
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity(); glTranslatef(0.0f, 0.0f, -4.0f);
        glRotatef(20.0f, 1.0f, 0.0f, 0.0f); glRotatef(rY, 0.0f, 1.0f, 0.0f);
        glBegin(GL_TRIANGLES);
        for (const auto& face : faces) {
            glColor3f(0.2f, 0.4f + (face.v1 * 0.1f), 0.8f);
            glVertex3f(vertices[face.v1].x, vertices[face.v1].y, vertices[face.v1].z);
            glVertex3f(vertices[face.v2].x, vertices[face.v2].y, vertices[face.v2].z);
            glVertex3f(vertices[face.v3].x, vertices[face.v3].y, vertices[face.v3].z);
        }
        glEnd();
        rY += 0.5f;
        glfwSwapBuffers(w); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}