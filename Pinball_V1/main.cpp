#define _USE_MATH_DEFINES
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <windows.h>
#include <vector>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

// ─── Fenster ──────────────────────────────────────────────────────────────
const int WIN_W = 900, WIN_H = 700;

// ─── Sound ─────────────────────────────────────────────────────────────────
#pragma comment(lib, "winmm.lib")

void playTone(int freq, int dur, int type) {
    static int id = 0;
    char name[32]; snprintf(name, 32, "pinball_snd_%d", id++ % 256);

    int sampleRate = 22050;
    int samples = (int)(sampleRate * dur / 1000.0f);
    if (samples < 10) samples = 10;

    // WAV header
    struct { char riff[4]; int len; char wave[4]; char fmt[4]; int flen;
        short tag, ch; int srate, bps; short align, bps2; } hdr;
    memcpy(hdr.riff, "RIFF", 4);
    hdr.len = 36 + samples * 2;
    memcpy(hdr.wave, "WAVE", 4);
    memcpy(hdr.fmt, "fmt ", 4);
    hdr.flen = 16; hdr.tag = 1; hdr.ch = 1; hdr.srate = sampleRate;
    hdr.bps = sampleRate * 2; hdr.align = 2; hdr.bps2 = 16;

    // Daten: Sinus + Obertöne = reicherer Sound
    short* data = new short[samples];
    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        float env = 1.0f - (float)i / samples;
        float s = 0;
        if (type == 0) { // Ping
            float f = freq + (800 - freq) * (1.0f - (float)i / samples);
            s = sinf(2 * M_PI * f * t) * env;
        } else if (type == 1) { // Bass
            s = sinf(2 * M_PI * freq * t) * env * 0.8f
              + sinf(2 * M_PI * freq * 0.5f * t) * env * 0.4f;
        } else if (type == 2) { // Noise (explosion)
            s = ((rand() % 2000 - 1000) / 1000.0f) * env * 0.5f;
        } else if (type == 3) { // Melody note
            s = sinf(2 * M_PI * freq * t) * env * 0.6f
              + sinf(2 * M_PI * freq * 2 * t) * env * 0.3f
              + sinf(2 * M_PI * freq * 3 * t) * env * 0.1f;
        }
        data[i] = (short)(s * 30000);
    }

    // Build WAV in memory
    int totalSize = sizeof(hdr) + samples * 2;
    char* wav = new char[totalSize];
    memcpy(wav, &hdr, sizeof(hdr));
    memcpy(wav + sizeof(hdr), data, samples * 2);
    delete[] data;

    // Write to TEMP folder
    char tmp[MAX_PATH]; GetTempPathA(MAX_PATH, tmp);
    char path[MAX_PATH]; snprintf(path, MAX_PATH, "%s%s.wav", tmp, name);
    FILE* f = fopen(path, "wb");
    if (f) { fwrite(wav, 1, totalSize, f); fclose(f); }
    delete[] wav;
    PlaySoundA(path, NULL, SND_ASYNC | SND_NOSTOP);
}

// ─── Screenshot ────────────────────────────────────────────────────────────
void screenshot() {
    static int counter = 0;
    char path[MAX_PATH]; snprintf(path, MAX_PATH, "pinball_%d.tga", counter++);
    FILE* f = fopen(path, "wb");
    if (!f) return;
    unsigned char* pixels = new unsigned char[WIN_W * WIN_H * 3];
    glReadPixels(0, 0, WIN_W, WIN_H, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    unsigned char header[18] = {0};
    header[2] = 2; header[12] = WIN_W & 255; header[13] = (WIN_W >> 8) & 255;
    header[14] = WIN_H & 255; header[15] = (WIN_H >> 8) & 255;
    header[16] = 24; header[17] = 0x20;
    fwrite(header, 1, 18, f);
    for (int y = WIN_H - 1; y >= 0; y--)
        fwrite(pixels + y * WIN_W * 3, 1, WIN_W * 3, f);
    delete[] pixels; fclose(f);
}

// ─── Font-Sheet Texturen ──────────────────────────────────────────────────
GLuint texFont;
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
    glPushMatrix(); glLoadIdentity();
    glOrtho(0, WIN_W, WIN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, texFont);
    glColor3f(1, 1, 1);
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1);
    for (const char* c = text; *c; c++) {
        int ch = (unsigned char)*c;
        float cx = (ch % 16) / 16.0f, cy = (ch / 16) / 16.0f, s = 1.0f / 16.0f;
        glBegin(GL_QUADS);
        glTexCoord2f(cx, cy);       glVertex2f(0, 0);
        glTexCoord2f(cx + s, cy);   glVertex2f(8, 0);
        glTexCoord2f(cx + s, cy + s); glVertex2f(8, 8);
        glTexCoord2f(cx, cy + s);   glVertex2f(0, 8);
        glEnd();
        glTranslatef(8, 0, 0);
    }
    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

// ─── Partikel ──────────────────────────────────────────────────────────────
struct Particle { float x, y, vx, vy, life, fade, r, g, b; };
const int MAX_P = 400;
Particle particles[MAX_P];
int pCount = 0;

void spawnParticles(float x, float y, int n, float r, float g, float b) {
    for (int i = 0; i < n && pCount < MAX_P; i++) {
        float a = (rand() % 360) * M_PI / 180.0f;
        float spd = 60 + (rand() % 250);
        particles[pCount++] = { x, y, cosf(a)*spd, sinf(a)*spd, 1.0f,
            0.015f + (rand() % 100) / 2000.0f,
            fminf(1,r+(rand()%50)/255.0f), fminf(1,g+(rand()%50)/255.0f), b };
    }
}

void updateParticles(float dt) {
    for (int i = 0; i < pCount; ) {
        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        particles[i].vy += 150.0f * dt;
        particles[i].life -= particles[i].fade;
        if (particles[i].life <= 0) particles[i] = particles[--pCount];
        else i++;
    }
}

void drawParticles() {
    if (!pCount) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    for (int i = 0; i < pCount; i++) {
        float s = 4.0f;
        glColor4f(particles[i].r, particles[i].g, particles[i].b, particles[i].life);
        glVertex2f(particles[i].x-s, particles[i].y-s);
        glVertex2f(particles[i].x+s, particles[i].y-s);
        glVertex2f(particles[i].x+s, particles[i].y+s);
        glVertex2f(particles[i].x-s, particles[i].y+s);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

// ─── Physik-Konstanten ────────────────────────────────────────────────────
const float GRAVITY_BASE = 500.0f;
const float BALL_R = 8.0f;
const float FLIPPER_LEN = 50.0f;
const float FLIPPER_W = 10.0f;
const float BUMPER_R = 20.0f;

// ─── Level-Definition ─────────────────────────────────────────────────────
struct Bumper { float x, y, r; int points; float cr, cg, cb; };
struct Level {
    const char* name;
    float r1, g1, b1, r2, g2, b2;
    int numBumpers;
    Bumper bumpers[8];
    float gravity;
    float speedMul;
    int entryScore;
};
Level levels[4];
int currentLevel = 0;
int unlockedLevel = 0;

void initLevels() {
    levels[0] = {"CLASSIC", 0.02f,0.02f,0.06f, 0.06f,0.06f,0.15f, 4, {
        {WIN_W/2-100, 250, BUMPER_R, 50, 0.2f,0.6f,1.0f},
        {WIN_W/2+100, 250, BUMPER_R, 50, 0.2f,0.6f,1.0f},
        {WIN_W/2, 180, BUMPER_R, 100, 1.0f,0.8f,0.2f},
        {WIN_W/2, 320, BUMPER_R, 100, 1.0f,0.8f,0.2f},
    }, GRAVITY_BASE, 1.0f, 500};

    levels[1] = {"SPACE", 0.04f,0.0f,0.08f, 0.1f,0.02f,0.15f, 6, {
        {WIN_W/2-140, 220, BUMPER_R, 50, 0.6f,0.2f,1.0f},
        {WIN_W/2+140, 220, BUMPER_R, 50, 0.6f,0.2f,1.0f},
        {WIN_W/2-70, 170, BUMPER_R, 100, 1.0f,0.3f,1.0f},
        {WIN_W/2+70, 170, BUMPER_R, 100, 1.0f,0.3f,1.0f},
        {WIN_W/2, 280, BUMPER_R, 75, 0.8f,0.4f,1.0f},
        {WIN_W/2, 350, BUMPER_R, 75, 0.8f,0.4f,1.0f},
    }, GRAVITY_BASE * 0.85f, 1.2f, 800};

    levels[2] = {"FIRE", 0.08f,0.0f,0.0f, 0.15f,0.04f,0.02f, 5, {
        {WIN_W/2-100, 200, BUMPER_R+5, 75, 1.0f,0.3f,0.1f},
        {WIN_W/2+100, 200, BUMPER_R+5, 75, 1.0f,0.3f,0.1f},
        {WIN_W/2, 150, BUMPER_R+8, 150, 1.0f,0.8f,0.0f},
        {WIN_W/2-60, 300, BUMPER_R, 100, 1.0f,0.5f,0.1f},
        {WIN_W/2+60, 300, BUMPER_R, 100, 1.0f,0.5f,0.1f},
    }, GRAVITY_BASE * 1.2f, 0.9f, 1200};

    levels[3] = {"BONUS", 0.0f,0.06f,0.04f, 0.02f,0.12f,0.08f, 8, {
        {WIN_W/2-160, 200, BUMPER_R, 50, 0.4f,1.0f,0.4f},
        {WIN_W/2+160, 200, BUMPER_R, 50, 0.4f,1.0f,0.4f},
        {WIN_W/2-80, 160, BUMPER_R, 75, 0.6f,1.0f,0.2f},
        {WIN_W/2+80, 160, BUMPER_R, 75, 0.6f,1.0f,0.2f},
        {WIN_W/2, 250, BUMPER_R, 100, 1.0f,0.9f,0.1f},
        {WIN_W/2-130, 300, BUMPER_R, 100, 1.0f,0.9f,0.1f},
        {WIN_W/2+130, 300, BUMPER_R, 100, 1.0f,0.9f,0.1f},
        {WIN_W/2, 370, BUMPER_R+3, 200, 1.0f,1.0f,0.5f},
    }, GRAVITY_BASE * 0.9f, 1.1f, 1500};
}

// ─── Spiel-Zustand ────────────────────────────────────────────────────────
enum State { ST_MENU, ST_PLAY, ST_LAUNCH, ST_DRAIN, ST_GAMEOVER, ST_LEVELSEL };
State state = ST_MENU;
int menuChoice = 0;
const char* menuItems[] = {"START GAME", "HIGHSCORES", "EXIT"};
const int menuItemsN = 3;

int lives = 3;
int score = 0;
int highscore[4] = {0};
int totalHighscore = 0;

// Ball
float bx, by, bvx, bvy;

// Flipper
float flipperL_angle = 30.0f;
float flipperR_angle = -30.0f;
bool flipperL_active = false;
bool flipperR_active = false;

// Plunger
float plungerPower = 0.0f;
bool plungerCharging = false;

// Drain timer
float drainTimer = 0.0f;

// Menu animation
float menuTime = 0.0f;
int hoveredItem = -1;

// ─── Geometrie-Hilfen ────────────────────────────────────────────────────
struct Line { float x1, y1, x2, y2; };
std::vector<Line> walls;

void buildWalls() {
    walls.clear();
    float pad = 40.0f;
    float top = 80.0f;
    float bottom = WIN_H - 30.0f;
    // Linke Wand
    walls.push_back({pad, top, pad, bottom});
    // Rechte Wand
    walls.push_back({WIN_W - pad, top, WIN_W - pad, bottom});
    // Obere Wand
    walls.push_back({pad, top, WIN_W - pad, top});
    // Schräge Wände oben links/rechts (trichter-artig)
    walls.push_back({pad, top, pad + 60, top + 60});
    walls.push_back({WIN_W - pad, top, WIN_W - pad - 60, top + 60});
    // Drain-Wände (schräge zum Loch in der Mitte)
    walls.push_back({pad, bottom, WIN_W/2 - 40, bottom + 20});
    walls.push_back({WIN_W - pad, bottom, WIN_W/2 + 40, bottom + 20});
}

// ─── Kollision ────────────────────────────────────────────────────────────
void reflect(float px, float py, float nx, float ny, float& vx, float& vy) {
    float dot = vx * nx + vy * ny;
    vx -= 2 * dot * nx;
    vy -= 2 * dot * ny;
}

float distPointLine(float px, float py, float x1, float y1, float x2, float y2, float& cx, float& cy) {
    float dx = x2 - x1, dy = y2 - y1;
    float len2 = dx*dx + dy*dy;
    if (len2 == 0) { cx = x1; cy = y1; return sqrtf((px-x1)*(px-x1)+(py-y1)*(py-y1)); }
    float t = fmaxf(0, fminf(1, ((px-x1)*dx + (py-y1)*dy) / len2));
    cx = x1 + t * dx; cy = y1 + t * dy;
    return sqrtf((px-cx)*(px-cx)+(py-cy)*(py-cy));
}

// ─── Spiel-Logik ─────────────────────────────────────────────────────────
void resetBall() {
    bx = WIN_W / 2.0f;
    by = WIN_H - 70.0f;
    bvx = 0; bvy = 0;
}

void launchBall() {
    float angle = -85.0f * M_PI / 180.0f;
    float power = 200 + plungerPower * 600;
    bvx = cosf(angle) * power;
    bvy = sinf(angle) * power;
    plungerPower = 0;
    plungerCharging = false;
}

bool checkCollisionCircle(float cx, float cy, float cr, float& px, float& py, float& vx, float& vy) {
    float dx = px - cx, dy = py - cy;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist < BALL_R + cr && dist > 0.001f) {
        float nx = dx / dist, ny = dy / dist;
        px = cx + nx * (BALL_R + cr + 1);
        py = cy + ny * (BALL_R + cr + 1);
        reflect(px, py, nx, ny, vx, vy);
        return true;
    }
    return false;
}

bool checkCollisionLine(float x1, float y1, float x2, float y2, float& px, float& py, float& vx, float& vy) {
    float cx, cy;
    float d = distPointLine(px, py, x1, y1, x2, y2, cx, cy);
    if (d < BALL_R + 2) {
        float nx = px - cx, ny = py - cy;
        float len = sqrtf(nx*nx+ny*ny);
        if (len > 0) { nx /= len; ny /= len; } else { nx = 0; ny = -1; }
        px = cx + nx * (BALL_R + 2);
        py = cy + ny * (BALL_R + 2);
        reflect(px, py, nx, ny, vx, vy);
        return true;
    }
    return false;
}

// ─── Rendern ──────────────────────────────────────────────────────────────
GLuint texBumper, texBallTex;

void createGameTextures() {
    // Bumper texture
    unsigned char bimg[64][64][3];
    for (int i = 0; i < 64; i++)
        for (int j = 0; j < 64; j++) {
            float dx = j-31.5f, dy = i-31.5f;
            float d = sqrtf(dx*dx+dy*dy);
            int v = (d < 30) ? 255 : 0;
            int center = (d < 12) ? 255 : 0;
            bimg[i][j][0] = v; bimg[i][j][1] = v; bimg[i][j][2] = center;
        }
    glGenTextures(1, &texBumper);
    glBindTexture(GL_TEXTURE_2D, texBumper);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, bimg);

    // Ball texture
    unsigned char blm[32][32][3];
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 32; j++) {
            float dx = j-15.5f, dy = i-15.5f;
            int v = (sqrtf(dx*dx+dy*dy) < 14) ? 255 : 0;
            blm[i][j][0] = v; blm[i][j][1] = v; blm[i][j][2] = v;
        }
    glGenTextures(1, &texBallTex);
    glBindTexture(GL_TEXTURE_2D, texBallTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, blm);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void drawFlipper(float x, float y, float angle, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(angle, 0, 0, 1);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor3f(r, g, b);
    glVertex2f(0, -FLIPPER_W/2);
    glVertex2f(FLIPPER_LEN, -FLIPPER_W/2);
    glVertex2f(FLIPPER_LEN, FLIPPER_W/2);
    glVertex2f(0, FLIPPER_W/2);
    glEnd();
    // Pivot circle
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(r*0.7f, g*0.7f, b*0.7f);
    glVertex2f(0, 0);
    for (int i = 0; i <= 12; i++) {
        float a = 2*M_PI*i/12;
        glVertex2f(cosf(a)*6, sinf(a)*6);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawBumper(float x, float y, float r, float cr, float cg, float cb) {
    glBindTexture(GL_TEXTURE_2D, texBumper);
    glColor3f(cr, cg, cb);
    float s = r / 20.0f * 32;
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5f, 0.5f); glVertex2f(x, y);
    for (int i = 0; i <= 20; i++) {
        float a = 2*M_PI*i/20;
        float tx = 0.5f + cosf(a)*0.5f, ty = 0.5f + sinf(a)*0.5f;
        glTexCoord2f(tx, ty);
        glVertex2f(x + cosf(a)*r, y + sinf(a)*r);
    }
    glEnd();
}

void drawBall() {
    glBindTexture(GL_TEXTURE_2D, texBallTex);
    glColor3f(1, 1, 1);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5f, 0.5f); glVertex2f(bx, by);
    for (int i = 0; i <= 20; i++) {
        float a = 2*M_PI*i/20;
        float tx = 0.5f + cosf(a)*0.5f, ty = 0.5f + sinf(a)*0.5f;
        glTexCoord2f(tx, ty);
        glVertex2f(bx + cosf(a)*BALL_R, by + sinf(a)*BALL_R);
    }
    glEnd();
}

void drawBackground(float r1, float g1, float b1, float r2, float g2, float b2) {
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor3f(r1, g1, b1); glVertex2f(0, 0); glVertex2f(WIN_W, 0);
    glColor3f(r2, g2, b2); glVertex2f(WIN_W, WIN_H); glVertex2f(0, WIN_H);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

bool pointInRect(float mx, float my, float x, float y, float w, float h) {
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

// ─── Hauptprogramm ────────────────────────────────────────────────────────
int main() {
    srand((unsigned)time(nullptr));
    initLevels();
    buildWalls();
    resetBall();

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* win = glfwCreateWindow(WIN_W, WIN_H, "Pinball V1", nullptr, nullptr);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_2D);
    createFontSheet();
    createGameTextures();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIN_W, WIN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    double last = glfwGetTime();
    int lastMouseL = GLFW_RELEASE, lastMouseR = GLFW_RELEASE, lastMouseM = GLFW_RELEASE;

    while (!glfwWindowShouldClose(win)) {
        double now = glfwGetTime();
        float dt = (float)fmin(now - last, 0.05);
        last = now;
        menuTime += dt;

        double mx, my; glfwGetCursorPos(win, &mx, &my);
        int mouseL = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT);
        int mouseR = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT);
        int mouseM = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE);

        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            if (state == ST_PLAY || state == ST_LAUNCH || state == ST_DRAIN)
                state = ST_MENU;
            else
                glfwSetWindowShouldClose(win, true);
        }
        static int lastF12 = GLFW_RELEASE;
        int f12 = glfwGetKey(win, GLFW_KEY_F12);
        if (f12 == GLFW_PRESS && lastF12 == GLFW_RELEASE) screenshot();
        lastF12 = f12;

        // ─── MENU ────────────────────────────────────────────────────────
        if (state == ST_MENU) {
            glClear(GL_COLOR_BUFFER_BIT);
            drawBackground(0.01f,0.01f,0.04f, 0.05f,0.02f,0.1f);

            // Animated background particles
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            for (int i = 0; i < 30; i++) {
                float px = (float)((i * 137 + (int)(menuTime*20)) % WIN_W);
                float py = (float)((i * 251 + (int)(menuTime*15)) % WIN_H);
                float alpha = 0.1f + 0.1f * sinf(menuTime * 2 + i);
                float sz = 3 + 2 * sinf(menuTime + i);
                glColor4f(0.3f, 0.5f + 0.3f*sinf(menuTime+i), 1.0f, alpha);
                glBegin(GL_QUADS);
                glVertex2f(px-sz, py-sz); glVertex2f(px+sz, py-sz);
                glVertex2f(px+sz, py+sz); glVertex2f(px-sz, py+sz);
                glEnd();
            }
            glDisable(GL_BLEND);
            glEnable(GL_TEXTURE_2D);

            // Title
            drawText("PINBALL", WIN_W/2 - 110, 120, 4.0f);
            drawText("V1", WIN_W/2 + 100, 140, 2.0f);

            // Menu items
            hoveredItem = -1;
            for (int i = 0; i < menuItemsN; i++) {
                float iy = 280 + i * 60;
                float iw = strlen(menuItems[i]) * 8 * 1.5f;
                float ix = WIN_W/2 - iw/2;
                bool hover = pointInRect((float)mx, (float)my, ix, iy-10, iw, 40);

                if (hover) hoveredItem = i;
                if (hover && mouseL == GLFW_PRESS && lastMouseL == GLFW_RELEASE) {
                    if (i == 0) { state = ST_LEVELSEL; playTone(600, 100, 0); }
                    if (i == 1) { /* show highscores - toggle display */ }
                    if (i == 2) { glfwSetWindowShouldClose(win, true); }
                }

                float cr = hover ? 1.0f : 0.6f;
                float cg = hover ? 0.8f : 0.5f;
                float cb = hover ? 0.3f : 0.6f;
                glColor3f(cr, cg, cb);
                drawText(menuItems[i], ix, iy, 1.5f);
            }
            // Keyboard shortcuts
            if (glfwGetKey(win, GLFW_KEY_1) == GLFW_PRESS) { state = ST_LEVELSEL; playTone(600, 100, 0); }
            if (glfwGetKey(win, GLFW_KEY_3) == GLFW_PRESS) glfwSetWindowShouldClose(win, true);

            // Controls hint
            glColor3f(0.4f, 0.4f, 0.6f);
            drawText("1=Play 3=Exit | MOUSE to select", WIN_W/2 - 180, WIN_H - 50, 0.8f);

            glfwSwapBuffers(win);
            glfwPollEvents();
            lastMouseL = mouseL; lastMouseR = mouseR; lastMouseM = mouseM;
            continue;
        }

        // ─── LEVEL SELECT ────────────────────────────────────────────────
        if (state == ST_LEVELSEL) {
            glClear(GL_COLOR_BUFFER_BIT);
            drawBackground(0.01f,0.01f,0.04f, 0.05f,0.02f,0.1f);

            drawText("SELECT LEVEL", WIN_W/2 - 100, 100, 2.0f);

            for (int i = 0; i < 4; i++) {
                float iy = 200 + i * 80;
                char buf[64];
                bool locked = i > unlockedLevel;
                snprintf(buf, sizeof(buf), "%s  [%d pts]", levels[i].name, levels[i].entryScore);
                float iw = strlen(buf) * 8 * 1.2f;
                float ix = WIN_W/2 - iw/2;
                bool hover = !locked && pointInRect((float)mx, (float)my, ix, iy-8, iw, 35);

                if (hover && mouseL == GLFW_PRESS && lastMouseL == GLFW_RELEASE) {
                    currentLevel = i;
                    score = 0; lives = 3;
                    resetBall();
                    state = ST_LAUNCH;
                    playTone(800, 150, 0);
                }

                float cr = locked ? 0.3f : (hover ? 1.0f : 0.8f);
                float cg = locked ? 0.3f : (hover ? 0.9f : 0.6f);
                float cb = locked ? 0.3f : (hover ? 0.5f : 0.4f);
                glColor3f(cr, cg, cb);
                drawText(buf, ix, iy, 1.2f);
                if (locked) {
                    drawText("[LOCKED]", ix + iw + 10, iy, 0.8f);
                }
            }

            // Keyboard shortcuts
            for (int i = 0; i < 4; i++) {
                bool locked = i > unlockedLevel;
                if (!locked && glfwGetKey(win, GLFW_KEY_1 + i) == GLFW_PRESS) {
                    currentLevel = i;
                    score = 0; lives = 3;
                    resetBall();
                    state = ST_LAUNCH;
                    playTone(800, 150, 0);
                }
            }

            glColor3f(0.4f, 0.4f, 0.6f);
            drawText("1-4 = select | ESC = back", 20, WIN_H - 30, 0.8f);

            glfwSwapBuffers(win);
            glfwPollEvents();
            lastMouseL = mouseL; lastMouseR = mouseR; lastMouseM = mouseM;
            continue;
        }

        // ─── GAMEOVER ────────────────────────────────────────────────────
        if (state == ST_GAMEOVER) {
            glClear(GL_COLOR_BUFFER_BIT);
            drawBackground(0.04f,0.01f,0.01f, 0.1f,0.02f,0.02f);

            drawText("GAME OVER", WIN_W/2 - 90, 200, 3.0f);
            char buf[64];
            snprintf(buf, sizeof(buf), "SCORE: %d", score);
            drawText(buf, WIN_W/2 - 60, 300, 1.8f);

            if (score > highscore[currentLevel]) {
                highscore[currentLevel] = score;
                drawText("NEW HIGHSCORE!", WIN_W/2 - 100, 360, 1.5f);
            }

            if (score >= levels[currentLevel].entryScore && currentLevel + 1 > unlockedLevel && currentLevel < 3) {
                unlockedLevel = currentLevel + 1;
                drawText("NEW LEVEL UNLOCKED!", WIN_W/2 - 120, 410, 1.3f);
            }

            drawText("CLICK to continue", WIN_W/2 - 90, 480, 1.0f);

            if (mouseL == GLFW_PRESS && lastMouseL == GLFW_RELEASE)
                state = ST_MENU;

            glfwSwapBuffers(win);
            glfwPollEvents();
            lastMouseL = mouseL; lastMouseR = mouseR; lastMouseM = mouseM;
            continue;
        }

        // ─── PLAY / LAUNCH / DRAIN ──────────────────────────────────────
        Level& lev = levels[currentLevel];
        float grav = lev.gravity;

        // Flipper update
        flipperL_active = (mouseL == GLFW_PRESS);
        flipperR_active = (mouseR == GLFW_PRESS);
        float targetL = flipperL_active ? -15.0f : 30.0f;
        float targetR = flipperR_active ? 15.0f : -30.0f;
        flipperL_angle += (targetL - flipperL_angle) * 10 * dt;
        flipperR_angle += (targetR - flipperR_angle) * 10 * dt;

        // Plunger
        plungerCharging = (mouseM == GLFW_PRESS && state == ST_LAUNCH);
        if (plungerCharging) plungerPower = fminf(1, plungerPower + dt * 1.5f);
        if (mouseM == GLFW_RELEASE && lastMouseM == GLFW_PRESS && state == ST_LAUNCH)
            launchBall();

        if (state == ST_LAUNCH && plungerPower <= 0 && bvy == 0 && bvx == 0)
            { /* waiting for plunger */ }
        else if (state == ST_LAUNCH && (bvx != 0 || bvy != 0))
            state = ST_PLAY;
        // Keyboard launch
        if (state == ST_LAUNCH && glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS && bvx == 0 && bvy == 0)
            { bvy = -400; plungerPower = 0; state = ST_PLAY; playTone(500, 200, 1); }

        // Physics
        if (state == ST_PLAY) {
            bvy += grav * dt;
            bx += bvx * dt;
            by += bvy * dt;

            // Wall collisions
            for (auto& w : walls)
                checkCollisionLine(w.x1, w.y1, w.x2, w.y2, bx, by, bvx, bvy);

            // Bumper collisions
            for (int i = 0; i < lev.numBumpers; i++) {
                Bumper& b = lev.bumpers[i];
                if (checkCollisionCircle(b.x, b.y, b.r, bx, by, bvx, bvy)) {
                    // Push ball away harder
                    float dx = bx - b.x, dy = by - b.y;
                    float d = sqrtf(dx*dx+dy*dy);
                    if (d > 0) { bx += dx/d * 10; by += dy/d * 10; }
                    bvx *= 1.1f; bvy *= 1.1f;
                    score += b.points;
                    spawnParticles(b.x, b.y, 20, b.cr, b.cg, b.cb);
                    playTone(600 + b.points, 80, 0);
                }
            }

            // Flipper collisions
            float fxL = 50, fyL = WIN_H - 50;
            float fxR = WIN_W - 50, fyR = WIN_H - 50;
            float exL = fxL + cosf(flipperL_angle * M_PI / 180) * FLIPPER_LEN;
            float eyL = fyL + sinf(flipperL_angle * M_PI / 180) * FLIPPER_LEN;
            float exR = fxR + cosf(flipperR_angle * M_PI / 180) * FLIPPER_LEN;
            float eyR = fyR + sinf(flipperR_angle * M_PI / 180) * FLIPPER_LEN;

            if (checkCollisionLine(fxL, fyL, exL, eyL, bx, by, bvx, bvy)) { playTone(300, 50, 1); }
            if (checkCollisionLine(fxR, fyR, exR, eyR, bx, by, bvx, bvy)) { playTone(300, 50, 1); }

            // Drain check
            if (by > WIN_H + 50) {
                lives--;
                playTone(200, 300, 1);
                if (lives <= 0) {
                    state = ST_GAMEOVER;
                } else {
                    state = ST_DRAIN;
                    drainTimer = 2.0f;
                }
            }
        }

        if (state == ST_DRAIN) {
            drainTimer -= dt;
            if (drainTimer <= 0) { resetBall(); state = ST_LAUNCH; plungerPower = 0; }
        }

        updateParticles(dt);

        // ─── RENDER ─────────────────────────────────────────────────────
        glClear(GL_COLOR_BUFFER_BIT);
        drawBackground(lev.r1, lev.g1, lev.b1, lev.r2, lev.g2, lev.b2);

        // Walls
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.3f, 0.3f, 0.5f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        for (auto& w : walls) {
            glVertex2f(w.x1, w.y1); glVertex2f(w.x2, w.y2);
        }
        glEnd();

        // Drain area
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.3f, 0.1f, 0.1f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(WIN_W/2 - 40, WIN_H - 10);
        glVertex2f(WIN_W/2 + 40, WIN_H - 10);
        glVertex2f(WIN_W/2 + 60, WIN_H + 10);
        glVertex2f(WIN_W/2 - 60, WIN_H + 10);
        glEnd();
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        // Bumpers
        for (int i = 0; i < lev.numBumpers; i++) {
            Bumper& b = lev.bumpers[i];
            float pulse = 1.0f + 0.1f * sinf(menuTime * 3 + i);
            drawBumper(b.x, b.y, b.r * pulse, b.cr, b.cg, b.cb);
        }

        // Flippers
        drawFlipper(50, WIN_H - 50, flipperL_angle, 0.8f, 0.3f, 0.3f);
        drawFlipper(WIN_W - 50, WIN_H - 50, flipperR_angle, 0.3f, 0.3f, 0.8f);

        // Ball (only in play/launch)
        if (state != ST_GAMEOVER)
            drawBall();

        // Particles
        drawParticles();

        // Plunger indicator
        if (state == ST_LAUNCH) {
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            float barH = plungerPower * 100;
            glColor4f(1.0f - plungerPower, plungerPower, 0, 0.8f);
            glBegin(GL_QUADS);
            glVertex2f(WIN_W/2 - 8, WIN_H - 50);
            glVertex2f(WIN_W/2 + 8, WIN_H - 50);
            glVertex2f(WIN_W/2 + 8, WIN_H - 50 - barH);
            glVertex2f(WIN_W/2 - 8, WIN_H - 50 - barH);
            glEnd();
            glDisable(GL_BLEND);
            glEnable(GL_TEXTURE_2D);
            drawText("HOLD MIDDLE MOUSE = PLUNGER", WIN_W/2 - 150, WIN_H - 20, 0.8f);
        }

        // HUD
        char hud[128];
        snprintf(hud, sizeof(hud), "%s | SCORE: %d | LIVES: %d",
            levels[currentLevel].name, score, lives);
        drawText(hud, 20, 20, 1.0f);

        if (state == ST_DRAIN) {
            char db[64]; snprintf(db, sizeof(db), "BALL LOST! %d lives left", lives);
            drawText(db, WIN_W/2 - 100, WIN_H/2, 1.5f);
        }

        glfwSwapBuffers(win);
        glfwPollEvents();
        lastMouseL = mouseL; lastMouseR = mouseR; lastMouseM = mouseM;
    }

    glDeleteTextures(1, &texFont);
    glDeleteTextures(1, &texBumper);
    glDeleteTextures(1, &texBallTex);
    glfwTerminate();
    return 0;
}
