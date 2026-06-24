#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>

// ─── Spielkonstanten ──────────────────────────────────────────────────────────
const int WIN_W = 800, WIN_H = 600;
const float PADDLE_W = 12.0f, PADDLE_H = 80.0f;
const float BALL_R = 8.0f;
const float SPEED = 350.0f;
const float AI_SPEED = 250.0f;

int main() {
    srand((unsigned)time(nullptr));

    float p1y = WIN_H / 2.0f - PADDLE_H / 2.0f;
    float p2y = WIN_H / 2.0f - PADDLE_H / 2.0f;
    float bx = WIN_W / 2.0f, by = WIN_H / 2.0f;
    float bvx = (rand() % 2 ? 1 : -1) * SPEED;
    float bvy = (rand() % 100 - 50) * 0.3f;
    int score1 = 0, score2 = 0;

    if (!glfwInit()) return -1;
    GLFWwindow* win = glfwCreateWindow(WIN_W, WIN_H, "Pong", nullptr, nullptr);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIN_W, WIN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    double last = glfwGetTime();

    while (!glfwWindowShouldClose(win)) {
        double now = glfwGetTime();
        float dt = (float)fmin(now - last, 0.05);
        last = now;

        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(win, true);

        // Steuerung Player 1 (W/S)
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
            p1y = fmaxf(0, p1y - SPEED * dt);
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
            p1y = fminf(WIN_H - PADDLE_H, p1y + SPEED * dt);

        // KI Player 2
        float mid2 = p2y + PADDLE_H / 2;
        if (mid2 < by - 5) p2y = fminf(WIN_H - PADDLE_H, p2y + AI_SPEED * dt);
        if (mid2 > by + 5) p2y = fmaxf(0, p2y - AI_SPEED * dt);

        // Ball Bewegung
        bx += bvx * dt;
        by += bvy * dt;

        // Wände oben/unten
        if (by - BALL_R <= 0) { by = BALL_R; bvy = fabsf(bvy); }
        if (by + BALL_R >= WIN_H) { by = WIN_H - BALL_R; bvy = -fabsf(bvy); }

        // Paddle 1 (links)
        if (bx - BALL_R <= PADDLE_W && by >= p1y && by <= p1y + PADDLE_H) {
            bx = PADDLE_W + BALL_R;
            bvx = fabsf(bvx);
            float offset = (by - (p1y + PADDLE_H / 2)) / (PADDLE_H / 2);
            bvy = offset * SPEED * 0.8f;
        }

        // Paddle 2 (rechts)
        if (bx + BALL_R >= WIN_W - PADDLE_W && by >= p2y && by <= p2y + PADDLE_H) {
            bx = WIN_W - PADDLE_W - BALL_R;
            bvx = -fabsf(bvx);
            float offset = (by - (p2y + PADDLE_H / 2)) / (PADDLE_H / 2);
            bvy = offset * SPEED * 0.8f;
        }

        // Tor
        if (bx < 0) { score2++; bx = WIN_W / 2; by = WIN_H / 2; bvx = SPEED; bvy = (rand() % 100 - 50) * 0.3f; }
        if (bx > WIN_W) { score1++; bx = WIN_W / 2; by = WIN_H / 2; bvx = -SPEED; bvy = (rand() % 100 - 50) * 0.3f; }

        // Fenstertitel mit Punktestand
        std::string title = "Pong  " + std::to_string(score1) + " : " + std::to_string(score2);
        glfwSetWindowTitle(win, title.c_str());

        // Rendern
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 1);

        // Mittellinie
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_LINES);
        glVertex2f(WIN_W / 2, 0); glVertex2f(WIN_W / 2, WIN_H);
        glEnd();

        // Schläger
        glColor3f(1, 1, 1);
        glBegin(GL_QUADS);
        glVertex2f(0, p1y); glVertex2f(PADDLE_W, p1y);
        glVertex2f(PADDLE_W, p1y + PADDLE_H); glVertex2f(0, p1y + PADDLE_H);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2f(WIN_W - PADDLE_W, p2y); glVertex2f(WIN_W, p2y);
        glVertex2f(WIN_W, p2y + PADDLE_H); glVertex2f(WIN_W - PADDLE_W, p2y + PADDLE_H);
        glEnd();

        // Ball
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(bx, by);
        for (int i = 0; i <= 20; ++i) {
            float a = 6.283185f * i / 20;
            glVertex2f(bx + cosf(a) * BALL_R, by + sinf(a) * BALL_R);
        }
        glEnd();

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
