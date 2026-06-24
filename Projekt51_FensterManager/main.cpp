#include <iostream>
#include "WindowManager.h"

static WindowManager* g_wm = nullptr;

static void onResize(GLFWwindow*, int w, int h) { if (g_wm) g_wm->onResize(w, h); }
static void onMouseButton(GLFWwindow*, int b, int a, int m) { if (g_wm) g_wm->onMouseButton(b, a, m); }
static void onCursorPos(GLFWwindow*, double x, double y) { if (g_wm) g_wm->onCursorPos(x, y); }
static void onScroll(GLFWwindow*, double dx, double dy) { if (g_wm) g_wm->onScroll(dx, dy); }
static void onKey(GLFWwindow*, int k, int s, int a, int m) { if (g_wm) g_wm->onKey(k, s, a, m); }

int main() {
    if (!glfwInit()) { std::cerr << "glfwInit failed\n"; return -1; }

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Projekt 51 - FensterManager", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) { std::cerr << "glewInit failed\n"; return -1; }

    WindowManager wm(window);
    g_wm = &wm;

    glfwSetWindowSizeCallback(window, onResize);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onCursorPos);
    glfwSetScrollCallback(window, onScroll);
    glfwSetKeyCallback(window, onKey);

    wm.init();

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    wm.onResize(fbW, fbH);

    glfwSwapInterval(1);
    double prev = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        wm.update((float)(now - prev));
        prev = now;

        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        wm.render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    g_wm = nullptr;
    glfwTerminate();
    return 0;
}
