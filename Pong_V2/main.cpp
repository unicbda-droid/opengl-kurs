#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdio>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

// Fenster
const int WIN_W = 800, WIN_H = 600;

// Spieler
const float PADDLE_W = 14.0f, PADDLE_H = 90.0f;
const float PADDLE_SPEED = 420.0f;

// Ball
const float BALL_R = 10.0f;
const float BASE_SPEED = 360.0f;
float ballSpeed = BASE_SPEED;

// Spielstand
int score1 = 0, score2 = 0;
const int WINNING_SCORE = 7;

// Spieler-Positionen
float p1y = WIN_H / 2.0f - PADDLE_H / 2.0f;
float p2y = WIN_H / 2.0f - PADDLE_H / 2.0f;

// Ball-Position & Geschwindigkeit
float bx, by, bvx, bvy;

// KI
float kiError = 0.0f;
float kiReactionTime = 0.0f;

// Partikel-System
struct Particle {
    float x, y, vx, vy, life, fade, r, g, b;
};
const int MAX_P = 300;
Particle particles[MAX_P];
int pCount = 0;

void spawnParticles(float x, float y, int count, float r, float g, float b) {
    for (int i = 0; i < count && pCount < MAX_P; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 50.0f + (rand() % 200);
        particles[pCount++] = { x, y,
            cosf(angle) * speed, sinf(angle) * speed,
            1.0f, 0.02f + (rand() % 100) / 2000.0f,
            fminf(1.0f, r + (rand() % 50) / 255.0f),
            fminf(1.0f, g + (rand() % 50) / 255.0f),
            b };
    }
}

void updateParticles(float dt) {
    for (int i = 0; i < pCount; ) {
        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        particles[i].vy += 120.0f * dt;
        particles[i].life -= particles[i].fade;
        if (particles[i].life <= 0.0f)
            particles[i] = particles[--pCount];
        else i++;
    }
}

void drawParticles() {
    if (pCount == 0) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    for (int i = 0; i < pCount; i++) {
        float s = 4.0f;
        glColor4f(particles[i].r, particles[i].g, particles[i].b, particles[i].life);
        glVertex2f(particles[i].x - s, particles[i].y - s);
        glVertex2f(particles[i].x + s, particles[i].y - s);
        glVertex2f(particles[i].x + s, particles[i].y + s);
        glVertex2f(particles[i].x - s, particles[i].y + s);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

// Texturen
GLuint texPaddle, texBall, texFont;

void createPaddleTexture() {
    unsigned char img[64][64][3];
    for (int i = 0; i < 64; i++)
        for (int j = 0; j < 64; j++) {
            bool c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));
            img[i][j][0] = c ? 255 : 180;
            img[i][j][1] = c ? 100 : 40;
            img[i][j][2] = c ? 40  : 20;
        }
    glGenTextures(1, &texPaddle);
    glBindTexture(GL_TEXTURE_2D, texPaddle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
}

void createBallTexture() {
    unsigned char img[32][32][3];
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 32; j++) {
            float dx = j - 15.5f, dy = i - 15.5f;
            float d = sqrtf(dx * dx + dy * dy);
            int v = (d < 15.0f) ? 255 : 0;
            img[i][j][0] = v; img[i][j][1] = v; img[i][j][2] = v;
        }
    glGenTextures(1, &texBall);
    glBindTexture(GL_TEXTURE_2D, texBall);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
}

void createFontSheet() {
    unsigned char img[128][128][3] = {0};
    for (int cy = 0; cy < 16; cy++)
        for (int cx = 0; cx < 16; cx++)
            for (int i = 1; i < 7; i++) {
                int px = cx * 8 + i, py = cy * 8 + 4;
                img[py][px][0] = 255; img[py][px][1] = 255; img[py][px][2] = 255;
                px = cx * 8 + 4; py = cy * 8 + i;
                img[py][px][0] = 255; img[py][px][1] = 255; img[py][px][2] = 255;
            }
    glGenTextures(1, &texFont);
    glBindTexture(GL_TEXTURE_2D, texFont);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
}

void drawText(const char* text, float x, float y, float scale) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIN_W, WIN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, texFont);
    glColor3f(1, 1, 1);

    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1);

    for (const char* c = text; *c; c++) {
        int ch = (unsigned char)*c;
        float cx = (ch % 16) / 16.0f;
        float cy = (ch / 16) / 16.0f;
        float s = 1.0f / 16.0f;
        glBegin(GL_QUADS);
        glTexCoord2f(cx, cy);          glVertex2f(0, 0);
        glTexCoord2f(cx + s, cy);      glVertex2f(8, 0);
        glTexCoord2f(cx + s, cy + s);  glVertex2f(8, 8);
        glTexCoord2f(cx, cy + s);      glVertex2f(0, 8);
        glEnd();
        glTranslatef(8, 0, 0);
    }

    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void resetBall(int direction) {
    bx = WIN_W / 2.0f;
    by = WIN_H / 2.0f;
    float speed = ballSpeed;
    bvx = direction * speed;
    bvy = (rand() % 200 - 100) * 0.3f;
    if (bvy == 0) bvy = 30.0f;
}

int main() {
    srand((unsigned)time(nullptr));

    resetBall(rand() % 2 ? 1 : -1);

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* win = glfwCreateWindow(WIN_W, WIN_H, "Pong V2", nullptr, nullptr);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_2D);
    createPaddleTexture();
    createBallTexture();
    createFontSheet();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIN_W, WIN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    double last = glfwGetTime();
    float goalPause = 0.0f;
    bool gameOver = false;

    while (!glfwWindowShouldClose(win)) {
        double now = glfwGetTime();
        float dt = (float)fmin(now - last, 0.05);
        last = now;

        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(win, true);

        if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (gameOver || goalPause > 0.0f) {
                score1 = 0; score2 = 0;
                ballSpeed = BASE_SPEED;
                resetBall(1);
                goalPause = 0.0f;
                gameOver = false;
                pCount = 0;
            }
        }

        if (goalPause > 0.0f) {
            goalPause -= dt;
            if (goalPause <= 0.0f) {
                resetBall((score1 > score2 || (score1 == score2 && rand() % 2)) ? 1 : -1);
            }
        }

        if (!gameOver && goalPause <= 0.0f) {
            // Player 1
            if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
                p1y = fmaxf(0, p1y - PADDLE_SPEED * dt);
            if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
                p1y = fminf(WIN_H - PADDLE_H, p1y + PADDLE_SPEED * dt);

            // KI Player 2
            kiReactionTime -= dt;
            if (kiReactionTime <= 0.0f) {
                kiError = (rand() % 200 - 100) * 0.5f;
                kiReactionTime = 0.05f + (rand() % 100) / 500.0f;
            }
            float target = by + kiError - PADDLE_H / 2.0f;
            if (p2y + PADDLE_H / 2 < by - 10)
                p2y = fminf(WIN_H - PADDLE_H, p2y + fabsf(bvx) * 0.7f * dt);
            else if (p2y + PADDLE_H / 2 > by + 10)
                p2y = fmaxf(0, p2y - fabsf(bvx) * 0.7f * dt);

            // Ball
            bx += bvx * dt;
            by += bvy * dt;

            // Wände oben/unten
            if (by - BALL_R <= 0) { by = BALL_R; bvy = fabsf(bvy); }
            if (by + BALL_R >= WIN_H) { by = WIN_H - BALL_R; bvy = -fabsf(bvy); }

            // Paddle 1 (links)
            if (bvx < 0 && bx - BALL_R <= PADDLE_W + 4 && by >= p1y && by <= p1y + PADDLE_H) {
                bx = PADDLE_W + BALL_R + 4;
                bvx = fabsf(bvx);
                float offset = (by - (p1y + PADDLE_H / 2)) / (PADDLE_H / 2);
                bvy = offset * ballSpeed * 0.7f;
                ballSpeed += 5.0f;
                spawnParticles(bx, by, 15, 1.0f, 1.0f, 0.5f);
            }

            // Paddle 2 (rechts)
            if (bvx > 0 && bx + BALL_R >= WIN_W - PADDLE_W - 4 && by >= p2y && by <= p2y + PADDLE_H) {
                bx = WIN_W - PADDLE_W - BALL_R - 4;
                bvx = -fabsf(bvx);
                float offset = (by - (p2y + PADDLE_H / 2)) / (PADDLE_H / 2);
                bvy = offset * ballSpeed * 0.7f;
                ballSpeed += 5.0f;
                spawnParticles(bx, by, 15, 1.0f, 0.3f, 0.8f);
            }

            // Tor
            if (bx < -BALL_R) {
                score2++;
                spawnParticles(0, by, 50, 1.0f, 0.3f, 0.8f);
                goalPause = 1.5f;
                if (score2 >= WINNING_SCORE) gameOver = true;
            }
            if (bx > WIN_W + BALL_R) {
                score1++;
                spawnParticles(WIN_W, by, 50, 1.0f, 1.0f, 0.5f);
                goalPause = 1.5f;
                if (score1 >= WINNING_SCORE) gameOver = true;
            }

            updateParticles(dt);
        }

        // ---- RENDERN ----
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);

        // Hintergrund-Gradient
        glBegin(GL_QUADS);
        glColor3f(0.02f, 0.02f, 0.06f);
        glVertex2f(0, 0); glVertex2f(WIN_W, 0);
        glColor3f(0.06f, 0.06f, 0.15f);
        glVertex2f(WIN_W, WIN_H); glVertex2f(0, WIN_H);
        glEnd();

        // Mittellinie (gestrichelt)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.3f, 0.3f, 0.5f, 0.5f);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0x00FF);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(WIN_W / 2, 0); glVertex2f(WIN_W / 2, WIN_H);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        // Schläger Player 1
        glBindTexture(GL_TEXTURE_2D, texPaddle);
        glColor3f(1, 1, 1);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(4, p1y);
        glTexCoord2f(1, 0); glVertex2f(4 + PADDLE_W, p1y);
        glTexCoord2f(1, 1); glVertex2f(4 + PADDLE_W, p1y + PADDLE_H);
        glTexCoord2f(0, 1); glVertex2f(4, p1y + PADDLE_H);
        glEnd();

        // Schläger Player 2
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(WIN_W - 4 - PADDLE_W, p2y);
        glTexCoord2f(1, 0); glVertex2f(WIN_W - 4, p2y);
        glTexCoord2f(1, 1); glVertex2f(WIN_W - 4, p2y + PADDLE_H);
        glTexCoord2f(0, 1); glVertex2f(WIN_W - 4 - PADDLE_W, p2y + PADDLE_H);
        glEnd();

        // Ball (blinkt während Tor-Pause)
        if (goalPause <= 0.0f || fmodf(goalPause * 10, 1.0f) < 0.5f) {
            glBindTexture(GL_TEXTURE_2D, texBall);
            glBegin(GL_TRIANGLE_FAN);
            glTexCoord2f(0.5f, 0.5f);
            glVertex2f(bx, by);
            for (int i = 0; i <= 20; i++) {
                float a = 6.283185f * i / 20;
                float tx = 0.5f + cosf(a) * 0.5f;
                float ty = 0.5f + sinf(a) * 0.5f;
                glTexCoord2f(tx, ty);
                glVertex2f(bx + cosf(a) * BALL_R, by + sinf(a) * BALL_R);
            }
            glEnd();
        }

        // Partikel
        drawParticles();

        // HUD - Punktestand
        char scoreText[64];
        snprintf(scoreText, sizeof(scoreText), "%d : %d", score1, score2);
        drawText(scoreText, WIN_W / 2 - 40, 20, 2.5f);

        // Game Over
        if (gameOver) {
            const char* msg = (score1 > score2) ? "Player 1 WINS!" : "Player 2 WINS!";
            float msgW = strlen(msg) * 16.0f * 2.0f;
            drawText(msg, WIN_W / 2 - msgW / 2, WIN_H / 2 - 20, 2.0f);
            drawText("SPACE to restart", WIN_W / 2 - 80, WIN_H / 2 + 20, 1.2f);
        }

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteTextures(1, &texPaddle);
    glDeleteTextures(1, &texBall);
    glDeleteTextures(1, &texFont);
    glfwTerminate();
    return 0;
}
