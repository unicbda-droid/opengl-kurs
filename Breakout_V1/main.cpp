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

const int WIN_W = 900, WIN_H = 700;

#pragma comment(lib, "winmm.lib")

void playTone(int freq, int dur, int type) {
    static int id = 0;
    char name[32]; snprintf(name, 32, "bsnd_%d", id++ % 256);
    int sr = 22050, samples = (int)(sr * dur / 1000.0f);
    if (samples < 10) samples = 10;
    struct { char riff[4]; int len; char wave[4]; char fmt[4]; int flen;
        short tag, ch; int srate, bps; short align, bps2; } hdr;
    memcpy(hdr.riff, "RIFF", 4); hdr.len = 36 + samples * 2;
    memcpy(hdr.wave, "WAVE", 4); memcpy(hdr.fmt, "fmt ", 4);
    hdr.flen = 16; hdr.tag = 1; hdr.ch = 1; hdr.srate = sr;
    hdr.bps = sr * 2; hdr.align = 2; hdr.bps2 = 16;
    short* data = new short[samples];
    for (int i = 0; i < samples; i++) {
        float t = (float)i / sr;
        float env = 1.0f - (float)i / samples;
        float a = 2 * M_PI * freq * t;
        float s = 0;
        switch (type) {
            case 0: { // sine sweep down
                float f = freq + (1200 - freq) * (1 - (float)i / samples);
                s = sinf(2 * M_PI * f * t); break;
            }
            case 1: { // sine + sub
                s = sinf(a) * 0.8f + sinf(a * 0.5f) * 0.4f; break;
            }
            case 2: { // sine + overtones
                s = sinf(a) * 0.5f + sinf(a * 1.5f) * 0.3f + sinf(a * 2.0f) * 0.2f; break;
            }
            case 3: { // square
                s = (fmodf(freq * t, 1) < 0.5f) ? 1.0f : -1.0f; break;
            }
            case 4: { // triangle
                float ph = fmodf(freq * t, 1);
                s = 4 * fabsf(ph - 0.5f) - 1; break;
            }
            case 5: { // noise burst
                s = (float)(rand() % 20001 - 10000) / 10000.0f; break;
            }
            case 6: { // sine sweep up
                float f = freq + (2000 - freq) * (float)i / samples;
                s = sinf(2 * M_PI * f * t); break;
            }
            case 7: { // pluck (fast decay sine + noise)
                float env2 = expf(-6 * (float)i / samples);
                s = sinf(a) * 0.7f + 0.3f * (float)(rand() % 20001 - 10000) / 10000.0f;
                s *= env2; env = 1; break;
            }
        }
        data[i] = (short)(s * env * 30000);
    }
    int total = sizeof(hdr) + samples * 2;
    char* wav = new char[total];
    memcpy(wav, &hdr, sizeof(hdr));
    memcpy(wav + sizeof(hdr), data, samples * 2);
    delete[] data;
    char tmp[MAX_PATH]; GetTempPathA(MAX_PATH, tmp);
    char path[MAX_PATH]; snprintf(path, MAX_PATH, "%s%s.wav", tmp, name);
    FILE* f = fopen(path, "wb");
    if (f) { fwrite(wav, 1, total, f); fclose(f); }
    delete[] wav;
    PlaySoundA(path, NULL, SND_ASYNC | SND_NOSTOP);
    // cleanup old temp files occasionally
    static int clean = 0;
    if (++clean % 50 == 0) {
        WIN32_FIND_DATAA fd; HANDLE h;
        char pat[MAX_PATH]; snprintf(pat, MAX_PATH, "%sbsnd_*.wav", tmp);
        h = FindFirstFileA(pat, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            int count = 0; char full[MAX_PATH];
            do {
                snprintf(full, MAX_PATH, "%s%s", tmp, fd.cFileName);
                DeleteFileA(full);
                if (++count > 200) break;
            } while (FindNextFileA(h, &fd));
            FindClose(h);
        }
    }
}

const char* bgmPath = 0;
void stopBGM() {
    PlaySoundA(0, 0, 0);
    if (bgmPath) { DeleteFileA(bgmPath); delete[] bgmPath; bgmPath = 0; }
}
void playBGM(int level) {
    stopBGM();
    int sr = 22050, dur = 8000, samples = sr * dur / 1000;
    short* data = new short[samples];
    // note tables per level (semitones from root)
    const int scales[5][7] = {
        {0,2,4,5,7,9,11}, // C major
        {0,2,3,5,7,8,10}, // D minor
        {0,1,3,5,7,8,10}, // E phrygian
        {0,2,4,5,7,9,10}, // F# dorian
        {0,2,3,5,7,8,11}, // G harmonic minor
    };
    const float roots[5] = {261.63f, 293.66f, 329.63f, 369.99f, 392.00f}; // C4 D4 E4 F#4 G4
    float root = roots[level % 5];
    float beat = sr * 0.25f; // 16th note at 150bpm
    int beats = samples / (int)beat;
    for (int i = 0; i < samples; i++) {
        float t = (float)i / sr;
        int b = (int)(t * 4); // beat index
        int bi = b % 16; // 4-bar loop = 16 beats
        float s = 0;
        // Bass on beats 0,4,8,12 (quarter notes)
        if (bi == 0 || bi == 4 || bi == 8 || bi == 12) {
            float fb = root * ((bi % 8 == 0) ? 1 : 1.5f); // root or fifth
            s += 0.2f * (fmodf(fb * t * 2, 1) < 0.5f ? 1 : -1); // square wave bass
        }
        // Percussion on beats 2,6,10,14 (off-beats)
        if (bi == 2 || bi == 6 || bi == 10 || bi == 14) {
            float env = expf(-20 * fmodf(t * 4, 1));
            s += 0.15f * env * (float)(rand() % 20001 - 10000) / 10000.0f;
        }
        // Melody: random arpeggio notes from scale
        int noteIdx = ((bi / 2) % 7);
        float melody = root * powf(2, scales[level % 5][noteIdx] / 12.0f);
        float envM = 1 - fmodf(t * 4, 1) * 0.5f;
        float ph = fmodf(melody * t * 2, 1);
        s += 0.12f * envM * (4 * fabsf(ph - 0.5f) - 1); // triangle melody
        data[i] = (short)(s * 12000);
    }
    // Write WAV
    struct { char riff[4]; int len; char wave[4]; char fmt[4]; int flen;
        short tag, ch; int srate, bps; short align, bps2; } hdr;
    memcpy(hdr.riff, "RIFF", 4); hdr.len = 36 + samples * 2;
    memcpy(hdr.wave, "WAVE", 4); memcpy(hdr.fmt, "fmt ", 4);
    hdr.flen = 16; hdr.tag = 1; hdr.ch = 1; hdr.srate = sr;
    hdr.bps = sr * 2; hdr.align = 2; hdr.bps2 = 16;
    int total = sizeof(hdr) + samples * 2;
    char* wav = new char[total];
    memcpy(wav, &hdr, sizeof(hdr));
    memcpy(wav + sizeof(hdr), data, samples * 2);
    delete[] data;
    char tmp[MAX_PATH]; GetTempPathA(MAX_PATH, tmp);
    char path[MAX_PATH]; snprintf(path, MAX_PATH, "%sbgm.wav", tmp);
    FILE* f = fopen(path, "wb");
    if (f) { fwrite(wav, 1, total, f); fclose(f); }
    delete[] wav;
    bgmPath = new char[MAX_PATH]; strcpy((char*)bgmPath, path);
    PlaySoundA(path, NULL, SND_ASYNC | SND_LOOP);
}

GLuint texFont;
int combo = 0;
void createFontSheet() {
    unsigned char img[256][256][3] = {0};
    HDC dc = CreateCompatibleDC(0);
    HBITMAP bmp = CreateCompatibleBitmap(dc, 256, 256);
    SelectObject(dc, bmp);
    SetBkColor(dc, RGB(0,0,0));
    SetTextColor(dc, RGB(255,255,255));
    HFONT font = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        NONANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    SelectObject(dc, font);
    for (int ch = 0; ch < 256; ch++) {
        int cx = (ch % 16) * 16, cy = (ch / 16) * 16;
        char str[2] = {(char)ch, 0};
        TextOutA(dc, cx, cy, str, 1);
    }
    DeleteObject(font);
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = 256;
    bi.bmiHeader.biHeight = -256;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    GetDIBits(dc, bmp, 0, 256, img, &bi, DIB_RGB_COLORS);
    DeleteObject(bmp); DeleteDC(dc);
    glGenTextures(1, &texFont);
    glBindTexture(GL_TEXTURE_2D, texFont);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
}

void drawText(const char* text, float x, float y, float scale) {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0, WIN_W, WIN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, texFont);
    glTranslatef(x, y, 0); glScalef(scale, scale, 1);
    for (const char* c = text; *c; c++) {
        int ch = (unsigned char)*c;
        float cx = (ch % 16) / 16.0f, cy = (ch / 16) / 16.0f, s = 1.0f / 16.0f;
        glBegin(GL_QUADS);
        glTexCoord2f(cx, cy); glVertex2f(0, 0);
        glTexCoord2f(cx + s, cy); glVertex2f(16, 0);
        glTexCoord2f(cx + s, cy + s); glVertex2f(16, 16);
        glTexCoord2f(cx, cy + s); glVertex2f(0, 16);
        glEnd(); glTranslatef(16, 0, 0);
    }
    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

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

void spawnBrickParticles(float x, float y, int n, float r, float g, float b) {
    spawnParticles(x, y, n, r, g, b);
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
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE);
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
    glEnd(); glDisable(GL_BLEND); glEnable(GL_TEXTURE_2D);
}

void drawBackground(float tR,float tG,float tB, float bR,float bG,float bB) {
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor3f(tR,tG,tB); glVertex2f(0,0); glVertex2f(WIN_W,0);
    glColor3f(bR,bG,bB); glVertex2f(WIN_W,WIN_H); glVertex2f(0,WIN_H);
    glEnd(); glEnable(GL_TEXTURE_2D);
}

bool pointInRect(float mx, float my, float x, float y, float w, float h) {
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

enum GameState { ST_MENU, ST_LEVELSEL, ST_PLAY, ST_PAUSE, ST_GAMEOVER };
GameState state = ST_MENU;
int score = 0, lives = 10, currentLevel = 0, unlockedLevel = 0;
void addComboScore(int base) {
    int mult = 1 + combo;
    score += base * mult;
    combo++;
}
float menuTime = 0;
int highscore[5] = {0};

// Brick types
const int BT_EMPTY = 0, BT_NORMAL = 1, BT_HARD = 2, BT_UNBREAK = 3, BT_EXPLOSIVE = 4, BT_LAVA = 5, BT_ACID = 6;

struct Brick {
    int type, hp;
    bool alive;
};

const int GRID_COLS = 10, GRID_ROWS = 8;
const int BRICK_W = 64, BRICK_H = 24, BRICK_GAP = 4;
const int GRID_X = 50, GRID_Y = 60;

struct LevelData {
    const char* name;
    int entryScore;
    int grid[GRID_ROWS][GRID_COLS];
};

LevelData levels[5] = {
    {"CLASSIC", 0, {
        {1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0}
    }},
    {"FORTRESS", 200, {
        {2,1,1,2,6,1,2,1,1,2},
        {1,2,1,1,2,1,1,2,1,1},
        {6,1,2,1,1,1,2,1,1,6},
        {2,1,1,2,1,1,2,1,1,2},
        {1,2,6,1,2,1,1,2,1,1},
        {1,1,2,1,1,1,2,1,1,1},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0}
    }},
    {"EXPLOSIVE", 500, {
        {4,1,5,4,1,1,4,5,1,4},
        {1,4,1,1,4,1,1,4,1,1},
        {5,1,4,1,1,1,4,1,1,5},
        {4,1,1,4,1,1,4,1,1,4},
        {1,4,5,1,4,1,1,5,4,1},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0}
    }},
    {"IMPREGNABLE", 900, {
        {2,2,3,2,2,2,3,2,2,2},
        {2,3,2,2,3,2,2,3,2,2},
        {2,2,5,3,2,2,3,5,2,2},
        {2,3,2,2,3,2,2,3,2,2},
        {2,2,5,3,2,2,3,5,2,2},
        {2,2,3,2,2,2,3,2,2,2},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0}
    }},
    {"FINAL", 1500, {
        {1,2,3,4,5,6,1,2,3,4},
        {3,4,5,6,1,2,3,4,5,6},
        {5,6,1,2,3,4,5,6,1,2},
        {2,3,4,5,6,1,2,3,4,5},
        {4,5,6,1,2,3,4,5,6,1},
        {6,1,2,3,4,5,6,1,2,3},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0}
    }}
};

Brick bricks[GRID_ROWS][GRID_COLS];

// Lava drops
struct LavaDrop { float x, y, vy; bool active; };
const int MAX_LAVA = 100;
LavaDrop lavaDrops[MAX_LAVA];
int lavaCount = 0;

void initBricks(int level) {
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++) {
            int t = levels[level].grid[r][c];
            bricks[r][c].type = t;
            bricks[r][c].alive = (t != BT_EMPTY);
            bricks[r][c].hp = (t == BT_HARD) ? 2 : 1;
        }
}

void getBrickRect(int r, int c, float& x, float& y, float& w, float& h) {
    x = (float)(GRID_X + c * (BRICK_W + BRICK_GAP));
    y = (float)(GRID_Y + r * (BRICK_H + BRICK_GAP));
    w = (float)BRICK_W; h = (float)BRICK_H;
}

// Ball / Paddle
float paddleX = WIN_W/2, paddleW = 80, paddleH = 12, paddleY = 660;
float paddleWDefault = 80;
float paddleWideTimer = 0;
float paddleSpeed = 400;
float paddleSpeedTimer = 0;
float paddleSpeedBoost = 1.0f; // 1.0=normal, >1=faster, <1=slower

struct Ball {
    float x, y, vx, vy, r;
    bool active;
    bool burning;
    float burnTimer;
};

const int MAX_BALLS = 100;
Ball balls[MAX_BALLS];
int ballCount = 1;

float ballDefaultSpeed = 320;

void resetBall() {
    combo = 0;
    ballCount = 1;
    balls[0] = { paddleX, paddleY - 10, 0, 0, 6, true, false, 0 };
}

void launchBall() {
    if (ballCount > 0 && balls[0].vx == 0 && balls[0].vy == 0) {
        float angle = -0.5f + (rand() % 1000) / 1000.0f;
        float spd = ballDefaultSpeed;
        balls[0].vx = sinf(angle) * spd;
        balls[0].vy = -spd * cosf(angle);
    }
}

void addExtraBall() {
    if (ballCount < MAX_BALLS) {
        float bx = balls[0].x, by = balls[0].y;
        float angle = -1.0f + (rand() % 2000) / 1000.0f;
        float spd = ballDefaultSpeed * 0.9f;
        balls[ballCount++] = { bx, by, sinf(angle)*spd, -spd*cosf(angle), 6, true, false, 0 };
        playTone(800, 80, 7);
    }
}

struct PowerUp {
    float x, y, vy;
    int type; // 1=Wide, 2=Burning, 3=ExtraBall, 4=Speed+, 5=Speed-, 6=ClearLine, 7=Multi100
    bool active;
    float r, g, b;
};

const int MAX_PUPS = 20;
PowerUp powerups[MAX_PUPS];
int puCount = 0;

void spawnPowerUp(float x, float y) {
    if (puCount >= MAX_PUPS) return;
    if ((rand() % 100) >= 20) return; // 20% chance
    int t = 1 + (rand() % 8); // 1..8 (8 = extra life)
    float r=0,g=0,b=0;
    switch (t) {
        case 1: r=0.2f; g=0.4f; b=1.0f; break; // Wide = blue
        case 2: r=0.8f; g=0.2f; b=0.8f; break; // Burning = purple
        case 3: r=0.2f; g=0.8f; b=0.2f; break; // Extra = green
        case 4: r=1.0f; g=0.8f; b=0.2f; break; // Speed+ = gold
        case 5: r=0.4f; g=0.6f; b=0.8f; break; // Speed- = steel
        case 6: r=1.0f; g=1.0f; b=1.0f; break; // ClearLine = white
        case 7: r=1.0f; g=0.5f; b=0.0f; break; // Multi100 = orange
        case 8: r=1.0f; g=0.2f; b=0.6f; break; // Extra Life = pink
    }
    powerups[puCount++] = { x, y, 120, t, true, r, g, b };
}

bool checkCollisionCircleRect(Ball& ball, float rx, float ry, float rw, float rh) {
    float cx = ball.x, cy = ball.y, cr = ball.r;
    float nearX = fmaxf(rx, fminf(cx, rx + rw));
    float nearY = fmaxf(ry, fminf(cy, ry + rh));
    float dx = cx - nearX, dy = cy - nearY;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist < cr && dist > 0.001f) {
        // Reflect
        float nx = dx / dist, ny = dy / dist;
        ball.x = nearX + nx * (cr + 1);
        ball.y = nearY + ny * (cr + 1);
        float dot = ball.vx * nx + ball.vy * ny;
        ball.vx -= 2 * dot * nx;
        ball.vy -= 2 * dot * ny;
        return true;
    }
    return false;
}

void drawBrick(int r, int c) {
    Brick& b = bricks[r][c];
    if (!b.alive) return;
    float x, y, w, h; getBrickRect(r, c, x, y, w, h);
    float cr=0,cg=0,cb=0;
    switch (b.type) {
        case BT_NORMAL: cr=0.2f; cg=0.8f; cb=0.2f; break; // Green
        case BT_HARD:   cr=1.0f; cg=0.6f; cb=0.0f; break; // Orange
        case BT_UNBREAK: cr=0.3f; cg=0.3f; cb=0.3f; break; // Grey
        case BT_EXPLOSIVE: cr=1.0f; cg=0.2f; cb=0.2f; break; // Red
        case BT_LAVA: cr=0.9f+0.1f*sinf(menuTime*5); cg=0.4f; cb=0.0f; break; // Lava = pulsing orange
        case BT_ACID: cr=0.0f; cg=0.6f+0.2f*sinf(menuTime*3); cb=0.0f; break; // Acid = pulsing green
    }
    glDisable(GL_TEXTURE_2D);
    // Brick body
    glColor3f(cr, cg, cb);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();
    // Border
    glColor3f(cr*0.6f, cg*0.6f, cb*0.6f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();
    // HP indicator for hard bricks
    if (b.type == BT_HARD && b.hp == 2) {
        glColor3f(1,1,1);
        glBegin(GL_LINES);
        glVertex2f(x+4, y+h/2); glVertex2f(x+w-4, y+h/2);
        glEnd();
    }
    glEnable(GL_TEXTURE_2D);
}

void drawPaddle() {
    float w = paddleW;
    glDisable(GL_TEXTURE_2D);
    // Paddle body
    glColor3f(0.6f, 0.7f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(paddleX - w/2, paddleY);
    glVertex2f(paddleX + w/2, paddleY);
    glVertex2f(paddleX + w/2, paddleY + paddleH);
    glVertex2f(paddleX - w/2, paddleY + paddleH);
    glEnd();
    // Highlight
    glColor3f(0.8f, 0.9f, 1.0f);
    glBegin(GL_LINES);
    glVertex2f(paddleX - w/2 + 2, paddleY + 2);
    glVertex2f(paddleX + w/2 - 2, paddleY + 2);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void drawBall(Ball& b) {
    if (!b.active) return;
    glDisable(GL_TEXTURE_2D);
    int segs = 16;
    if (b.burning) {
        // Burning glow
        glColor4f(1, 0.3f, 0.1f, 0.3f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(b.x, b.y);
        for (int i = 0; i <= segs; i++) {
            float a = i * 2 * M_PI / segs;
            glVertex2f(b.x + cosf(a) * (b.r + 4), b.y + sinf(a) * (b.r + 4));
        }
        glEnd();
    }
    glColor3f(b.burning ? 1.0f : 0.8f, b.burning ? 0.8f : 0.8f, b.burning ? 0.2f : 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(b.x, b.y);
    for (int i = 0; i <= segs; i++) {
        float a = i * 2 * M_PI / segs;
        glVertex2f(b.x + cosf(a) * b.r, b.y + sinf(a) * b.r);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void drawPowerUps() {
    glDisable(GL_TEXTURE_2D);
    for (int i = 0; i < puCount; i++) {
        PowerUp& p = powerups[i];
        if (!p.active) continue;
        glColor3f(p.r, p.g, p.b);
        glBegin(GL_QUADS);
        glVertex2f(p.x-6, p.y-6); glVertex2f(p.x+6, p.y-6);
        glVertex2f(p.x+6, p.y+6); glVertex2f(p.x-6, p.y+6);
        glEnd();
        // Inner icon
        glColor3f(1,1,1);
        char icon = p.type == 1 ? 'W' : (p.type == 2 ? 'F' : (p.type == 3 ? 'E' :
            (p.type == 4 ? '+' : (p.type == 5 ? '-' : (p.type == 6 ? 'C' : (p.type == 7 ? 'M' : 'L'))))));
        glEnable(GL_TEXTURE_2D);
        glColor3f(1,1,1);
        char str[2] = {icon, 0};
        drawText(str, p.x-4, p.y-4, 1.0f);
        glDisable(GL_TEXTURE_2D);
    }
    glEnable(GL_TEXTURE_2D);
}

bool allBricksDestroyed() {
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            if (bricks[r][c].alive) return false;
    return true;
}

void spawnLavaDrop(float x, float y); // forward decl

void destroyNeighbors(int r, int c) {
    for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < GRID_ROWS && nc >= 0 && nc < GRID_COLS && bricks[nr][nc].alive) {
                Brick& nb = bricks[nr][nc];
                if (nb.type != BT_UNBREAK) {
                    nb.alive = false;
                    addComboScore(10);
                    float bx, by, bw, bh; getBrickRect(nr, nc, bx, by, bw, bh);
                    spawnBrickParticles(bx+bw/2, by+bh/2, 15, 1,0.8f,0.2f);
                    playTone(400, 60, 0);
                }
            }
        }
}

void destroyColumnBelow(int r, int c) {
    for (int nr = r + 1; nr < GRID_ROWS; nr++) {
        Brick& nb = bricks[nr][c];
        if (!nb.alive || nb.type == BT_UNBREAK) continue;
        nb.alive = false;
        addComboScore(10);
        float bx, by, bw, bh; getBrickRect(nr, c, bx, by, bw, bh);
        spawnBrickParticles(bx+bw/2, by+bh/2, 15, 0, 0.8f, 0.2f);
        playTone(300, 40, 0);
    }
}

void hitBrick(int r, int c) {
    Brick& b = bricks[r][c];
    if (!b.alive || b.type == BT_UNBREAK) return;
    float bx, by, bw, bh; getBrickRect(r, c, bx, by, bw, bh);
    b.hp--;
    if (b.hp <= 0) {
        b.alive = false;
        int pts = (b.type == BT_HARD) ? 25 : (b.type == BT_EXPLOSIVE) ? 20 : (b.type == BT_ACID) ? 15 : 10;
        addComboScore(pts);
        float pr = (b.type == BT_NORMAL) ? 0.2f : (b.type == BT_HARD) ? 1.0f : (b.type == BT_LAVA) ? 1.0f : (b.type == BT_ACID) ? 0.0f : 1.0f;
        float pg = (b.type == BT_NORMAL) ? 0.8f : (b.type == BT_HARD) ? 0.6f : (b.type == BT_LAVA) ? 0.4f : (b.type == BT_ACID) ? 0.8f : 0.2f;
        float pb = (b.type == BT_NORMAL) ? 0.2f : (b.type == BT_HARD) ? 0.0f : (b.type == BT_LAVA) ? 0.0f : (b.type == BT_ACID) ? 0.2f : 0.0f;
        spawnBrickParticles(bx+bw/2, by+bh/2,
            (b.type == BT_EXPLOSIVE) ? 30 : 15, pr, pg, pb);
        playTone(500 + pts*10, 80, 0);
        if (b.type == BT_EXPLOSIVE) destroyNeighbors(r, c);
        if (b.type == BT_ACID) destroyColumnBelow(r, c);
        spawnPowerUp(bx+bw/2, by+bh/2);
    } else {
        spawnBrickParticles(bx+bw/2, by+bh/2, 5, 1, 0.8f, 0);
        playTone(300, 50, 0);
    }
}

void brickCollision(Ball& ball) {
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++) {
            Brick& b = bricks[r][c];
            if (!b.alive) continue;
            float bx, by, bw, bh; getBrickRect(r, c, bx, by, bw, bh);
            if (checkCollisionCircleRect(ball, bx, by, bw, bh)) {
                if (b.type == BT_UNBREAK) {
                    playTone(200, 50, 1);
                } else {
                    // Lava bricks also get hit
                    if (b.type == BT_LAVA) {
                        for (int k = 0; k < 3; k++) spawnLavaDrop(bx, by);
                    }
                    if (ball.burning) {
                        b.alive = false;
                        int pts = (b.type == BT_HARD) ? 25 : (b.type == BT_EXPLOSIVE) ? 20 : (b.type == BT_LAVA) ? 15 : 10;
                        addComboScore(pts);
                        spawnBrickParticles(bx+bw/2, by+bh/2, 20, 1,0.5f,0);
                        playTone(700, 60, 2);
                        if (b.type == BT_EXPLOSIVE) destroyNeighbors(r, c);
                        if (b.type == BT_ACID) destroyColumnBelow(r, c);
                        spawnPowerUp(bx+bw/2, by+bh/2);
                    } else {
                        hitBrick(r, c);
                    }
                }
                return;
            }
        }
}

void paddleCollision(Ball& ball) {
    if (!ball.active) return;
    float px = paddleX - paddleW/2, py = paddleY;
    if (ball.vy > 0 && ball.y + ball.r >= py && ball.y - ball.r <= py + paddleH &&
        ball.x + ball.r >= px && ball.x - ball.r <= px + paddleW) {
        // Hit position determines reflection angle
        float rel = (ball.x - paddleX) / (paddleW / 2);
        rel = fmaxf(-1.0f, fminf(1.0f, rel));
        float angle = rel * 0.8f; // -0.8 to 0.8 radians
        if (fabsf(angle) < 0.15f) angle = (angle >= 0) ? 0.15f : -0.15f;
        float spd = sqrtf(ball.vx*ball.vx + ball.vy*ball.vy);
        if (spd < 100) spd = ballDefaultSpeed;
        ball.vx = sinf(angle) * spd;
        ball.vy = -fabsf(cosf(angle) * spd);
        ball.y = py - ball.r - 1;
        playTone(700, 80, 1);
    }
}

// ─── Lava Drops ────────────────────────────────────────────────────────────
void spawnLavaDrop(float x, float y) {
    if (lavaCount < MAX_LAVA) {
        float dx = x + 16 + (float)(rand() % 32);
        float dy = y + 24;
        float spd = 150.0f + (float)(rand() % 100);
        lavaDrops[lavaCount++] = { dx, dy, spd, true };
    }
}

void updateLavaDrops(float dt) {
    for (int i = 0; i < lavaCount; ) {
        if (!lavaDrops[i].active) { lavaDrops[i] = lavaDrops[--lavaCount]; continue; }
        lavaDrops[i].y += lavaDrops[i].vy * dt;
        if (lavaDrops[i].y > WIN_H + 20) { lavaDrops[i] = lavaDrops[--lavaCount]; continue; }
        if (lavaDrops[i].y >= paddleY - 6 && lavaDrops[i].y <= paddleY + paddleH + 6 &&
            lavaDrops[i].x >= paddleX - paddleW/2 - 6 && lavaDrops[i].x <= paddleX + paddleW/2 + 6) {
            lavaDrops[i].active = false;
            playTone(200, 100, 0);
        }
        i++;
    }
}

void drawLavaDrops() {
    glDisable(GL_TEXTURE_2D);
    for (int i = 0; i < lavaCount; i++) {
        if (!lavaDrops[i].active) continue;
        glColor3f(1, 0.3f, 0);
        glBegin(GL_QUADS);
        glVertex2f(lavaDrops[i].x-3, lavaDrops[i].y-3);
        glVertex2f(lavaDrops[i].x+3, lavaDrops[i].y-3);
        glVertex2f(lavaDrops[i].x+3, lavaDrops[i].y+3);
        glVertex2f(lavaDrops[i].x-3, lavaDrops[i].y+3);
        glEnd();
    }
    glEnable(GL_TEXTURE_2D);
}

// ─── Screenshot ────────────────────────────────────────────────────────────
void screenshot() {
    static int counter = 0;
    char path[MAX_PATH]; snprintf(path, MAX_PATH, "breakout_%d.tga", counter++);
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

int main() {
    srand((unsigned)time(nullptr));
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* win = glfwCreateWindow(WIN_W, WIN_H, "Breakout V1", nullptr, nullptr);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_2D);
    createFontSheet();

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0, WIN_W, WIN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    double last = glfwGetTime();
    int lastMouseL = GLFW_RELEASE;
    int lastKey[512];
    for (int i = 0; i < 512; i++) lastKey[i] = GLFW_RELEASE;

    const char* menuItems[] = {"PLAY", "HIGHSCORES", "EXIT"};
    const int menuItemsN = 3;

    state = ST_MENU;

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        double now = glfwGetTime();
        float dt = (float)fmin(now - last, 0.05);
        last = now;
        menuTime += dt;

        double mx, my; glfwGetCursorPos(win, &mx, &my);
        int mouseL = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT);

        // Edge-detection helper
        #define KEY_PRESSED(k) (glfwGetKey(win, (k)) == GLFW_PRESS && lastKey[(k)] == GLFW_RELEASE)

        if (KEY_PRESSED(GLFW_KEY_F12)) screenshot();
        if (KEY_PRESSED(GLFW_KEY_ESCAPE)) {
            if (state == ST_PLAY) state = ST_PAUSE;
            else if (state == ST_PAUSE) state = ST_PLAY;
            else glfwSetWindowShouldClose(win, true);
        }
        // ─── MENU ────────────────────────────────────────────────────────
        if (state == ST_MENU) {
            glClear(GL_COLOR_BUFFER_BIT);
            drawBackground(0.01f,0.01f,0.04f, 0.05f,0.02f,0.1f);

            glDisable(GL_TEXTURE_2D); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE);
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
            glDisable(GL_BLEND); glEnable(GL_TEXTURE_2D);

            glColor3f(1,1,1);
            drawText("BREAKOUT", WIN_W/2 - 130, 100, 4.0f);
            drawText("V1", WIN_W/2 + 130, 120, 2.0f);

            for (int i = 0; i < menuItemsN; i++) {
                float iy = 260 + i * 60;
                float iw = strlen(menuItems[i]) * 8 * 1.5f;
                float ix = WIN_W/2 - iw/2;
                bool hover = pointInRect((float)mx, (float)my, ix, iy-10, iw, 40);
                if (hover && mouseL == GLFW_PRESS && lastMouseL == GLFW_RELEASE) {
                    if (i == 0) { state = ST_LEVELSEL; playTone(600, 100, 0); }
                    if (i == 2) { glfwSetWindowShouldClose(win, true); }
                }
                float cr = hover ? 1.0f : 0.6f, cg = hover ? 0.8f : 0.5f, cb = hover ? 0.3f : 0.6f;
                glColor3f(cr, cg, cb);
                drawText(menuItems[i], ix, iy, 1.5f);
            }
            if (KEY_PRESSED(GLFW_KEY_1)) { state = ST_LEVELSEL; playTone(600, 100, 0); }
            if (KEY_PRESSED(GLFW_KEY_3)) glfwSetWindowShouldClose(win, true);

            glColor3f(0.4f, 0.4f, 0.6f);
            drawText("1=Play  3=Exit  |  MOUSE + ESC", WIN_W/2 - 180, WIN_H - 50, 0.8f);

            glfwSwapBuffers(win);
            lastMouseL = mouseL;
            lastKey[GLFW_KEY_1] = glfwGetKey(win, GLFW_KEY_1);
            lastKey[GLFW_KEY_3] = glfwGetKey(win, GLFW_KEY_3);
            continue;
        }

        // ─── LEVEL SELECT ────────────────────────────────────────────────
        if (state == ST_LEVELSEL) {
            glClear(GL_COLOR_BUFFER_BIT);
            drawBackground(0.01f,0.01f,0.04f, 0.05f,0.02f,0.1f);

            glColor3f(1,1,1);
            drawText("SELECT LEVEL", WIN_W/2 - 100, 100, 2.0f);

            for (int i = 0; i < 5; i++) {
                float iy = 180 + i * 70;
                char buf[64];
                bool locked = i > unlockedLevel;
                snprintf(buf, sizeof(buf), "%s  [%d pts]", levels[i].name, levels[i].entryScore);
                float iw = strlen(buf) * 8 * 1.2f;
                float ix = WIN_W/2 - iw/2;
                bool hover = !locked && pointInRect((float)mx, (float)my, ix, iy-8, iw, 35);
                if (hover && mouseL == GLFW_PRESS && lastMouseL == GLFW_RELEASE) {
                    currentLevel = i; score = 0; lives = 10;
                    initBricks(i); resetBall();
                    paddleW = paddleWDefault; paddleWideTimer = 0;
                    state = ST_PLAY; playBGM(currentLevel);
                }
                float cr = locked ? 0.3f : (hover ? 1.0f : 0.8f);
                float cg = locked ? 0.3f : (hover ? 0.9f : 0.6f);
                float cb = locked ? 0.3f : (hover ? 0.5f : 0.4f);
                glColor3f(cr, cg, cb);
                drawText(buf, ix, iy, 1.2f);
                if (locked) drawText("[LOCKED]", ix + iw + 10, iy, 0.8f);
            }
            for (int i = 0; i < 5; i++) {
                bool locked = i > unlockedLevel;
                if (!locked && KEY_PRESSED(GLFW_KEY_1 + i)) {
                    currentLevel = i; score = 0; lives = 10;
                    initBricks(i); resetBall();
                    paddleW = paddleWDefault; paddleWideTimer = 0;
                    puCount = 0;
                    state = ST_PLAY; playBGM(currentLevel);
                }
            }

            glColor3f(0.4f, 0.4f, 0.6f);
            drawText("1-5 = select  |  ESC = back", 20, WIN_H - 30, 0.8f);

            glfwSwapBuffers(win);
            lastMouseL = mouseL;
            for (int i = 0; i < 5; i++) lastKey[GLFW_KEY_1 + i] = glfwGetKey(win, GLFW_KEY_1 + i);
            continue;
        }

        // ─── GAME OVER ────────────────────────────────────────────────────
        if (state == ST_GAMEOVER) {
            glClear(GL_COLOR_BUFFER_BIT);
            drawBackground(0.04f,0.01f,0.01f, 0.1f,0.02f,0.02f);
            glColor3f(1,1,1);
            drawText("GAME OVER", WIN_W/2 - 110, 200, 3.0f);
            char buf[64];
            bool won = allBricksDestroyed();
            if (won) { glColor3f(0.2f,1,0.2f); drawText("LEVEL COMPLETE!", WIN_W/2 - 120, 280, 1.5f); }
            snprintf(buf, sizeof(buf), "SCORE: %d", score);
            drawText(buf, WIN_W/2 - 60, 340, 1.5f);
            if (score > highscore[currentLevel]) {
                highscore[currentLevel] = score;
                glColor3f(1,1,0);
                drawText("NEW HIGHSCORE!", WIN_W/2 - 100, 390, 1.3f);
            }
            glColor3f(0.6f,0.6f,0.8f);
            drawText("CLICK or SPACE to continue", WIN_W/2 - 120, 470, 1.0f);
            if (mouseL == GLFW_PRESS && lastMouseL == GLFW_RELEASE) state = ST_MENU;
            if (KEY_PRESSED(GLFW_KEY_SPACE)) state = ST_MENU;
            glfwSwapBuffers(win);
            lastMouseL = mouseL;
            lastKey[GLFW_KEY_SPACE] = glfwGetKey(win, GLFW_KEY_SPACE);
            continue;
        }

        // ─── PLAY ────────────────────────────────────────────────────────
        // Paddle movement
        paddleX = (float)fmax(20 + paddleW/2, fmin(WIN_W - 20 - paddleW/2, mx));
        float pspd = paddleSpeed * paddleSpeedBoost;
        if (glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS) paddleX -= pspd * dt;
        if (glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS) paddleX += pspd * dt;
        paddleX = fmaxf(paddleW/2, fminf(WIN_W - paddleW/2, paddleX));

        // Sticky ball follows paddle
        for (int i = 0; i < ballCount; i++) {
            if (balls[i].active && balls[i].vx == 0 && balls[i].vy == 0) {
                balls[i].x = paddleX;
                balls[i].y = paddleY - 10;
            }
        }

        // Launch with left click or space
        if (mouseL == GLFW_PRESS && lastMouseL == GLFW_RELEASE) launchBall();
        if (KEY_PRESSED(GLFW_KEY_SPACE)) launchBall();

        // Wide paddle timer
        if (paddleWideTimer > 0) {
            paddleWideTimer -= dt;
            if (paddleWideTimer <= 0) { paddleW = paddleWDefault; paddleWideTimer = 0; }
        }

        // Speed boost timer
        if (paddleSpeedTimer > 0) {
            paddleSpeedTimer -= dt;
            if (paddleSpeedTimer <= 0) { paddleSpeedBoost = 1.0f; paddleSpeedTimer = 0; }
        }

        // Burning ball timer
        for (int i = 0; i < ballCount; i++) {
            if (balls[i].active && balls[i].burning) {
                balls[i].burnTimer -= dt;
                if (balls[i].burnTimer <= 0) { balls[i].burning = false; balls[i].burnTimer = 0; }
            }
        }

        // PowerUp movement + collision
        for (int i = 0; i < puCount; i++) {
            if (!powerups[i].active) continue;
            powerups[i].y += powerups[i].vy * dt;
            if (powerups[i].y > WIN_H + 20) { powerups[i].active = false; continue; }
            // Catch with paddle
            float px = paddleX - paddleW/2, py = paddleY;
            if (powerups[i].x >= px && powerups[i].x <= px + paddleW &&
                powerups[i].y >= py && powerups[i].y <= py + paddleH) {
                powerups[i].active = false;
                switch (powerups[i].type) {
                    case 1: // Wide
                        paddleW = paddleWDefault * 1.5f;
                        paddleWideTimer = 10.0f;
                        playTone(500, 200, 1);
                        break;
                    case 2: // Burning
                        for (int b = 0; b < ballCount; b++) {
                            if (balls[b].active) { balls[b].burning = true; balls[b].burnTimer = 8.0f; }
                        }
                        playTone(900, 300, 2);
                        break;
                    case 3: // Extra Ball
                        addExtraBall();
                        break;
                    case 4: // Speed+
                        paddleSpeedBoost = 2.0f;
                        paddleSpeedTimer = 8.0f;
                        playTone(600, 200, 1);
                        break;
                    case 5: // Speed-
                        paddleSpeedBoost = 0.4f;
                        paddleSpeedTimer = 6.0f;
                        playTone(300, 200, 0);
                        break;
                    case 6: // Clear the Line — safe: hits each brick once
                        for (int r = 0; r < GRID_ROWS; r++)
                            for (int c = 0; c < GRID_COLS; c++) {
                                Brick& cb = bricks[r][c];
                                if (cb.alive && cb.type != BT_UNBREAK) {
                                    cb.hp--;
                                    if (cb.hp <= 0) {
                                        cb.alive = false;
                                        addComboScore(10);
                                    }
                                    float bx,by,bw,bh; getBrickRect(r,c,bx,by,bw,bh);
                                    int n = (cb.hp <= 0) ? 10 : 3;
                                    spawnBrickParticles(bx+bw/2,by+bh/2,n,1,1,1);
                                }
                            }
                        playTone(1000, 500, 2);
                        break;
                    case 7: // Multi100
                        if (ballCount == 1 && balls[0].active) {
                            float sx = balls[0].x, sy = balls[0].y;
                            int maxB = MAX_BALLS < 100 ? MAX_BALLS : 100;
                            for (int k = 0; k < maxB && ballCount < MAX_BALLS; k++) {
                                if (k == 0) continue; // keep original ball
                                float a = (rand() % 3600) / 10.0f * M_PI / 180.0f;
                                float spd = ballDefaultSpeed * (0.5f + (rand() % 1000) / 1000.0f);
                                balls[ballCount++] = { sx, sy, sinf(a)*spd, -fabsf(cosf(a)*spd), 6, true, false, 0 };
                            }
                        }
                        playTone(800, 400, 2);
                        break;
                    case 8: // Extra Life
                        lives++;
                        playTone(600, 300, 3);
                        break;
                }
            }
        }

        // Ball physics
        for (int i = 0; i < ballCount; i++) {
            Ball& ball = balls[i];
            if (!ball.active) continue;

            // Ball stuck on paddle - skip physics
            if (ball.vx == 0 && ball.vy == 0) continue;

            ball.x += ball.vx * dt;
            ball.y += ball.vy * dt;

            // Wall collision
            if (ball.x - ball.r < 0) { ball.x = ball.r; ball.vx = -ball.vx; }
            if (ball.x + ball.r > WIN_W) { ball.x = WIN_W - ball.r; ball.vx = -ball.vx; }
            if (ball.y - ball.r < 0) { ball.y = ball.r; ball.vy = -ball.vy; }

            // Brick collisions
            brickCollision(ball);

            // Paddle collision
            paddleCollision(ball);

            // Bottom = lost ball
            if (ball.y > WIN_H + 20) {
                ball.active = false;
            }
        }

        // Check if all balls lost
        bool anyActive = false;
        for (int i = 0; i < ballCount; i++)
            if (balls[i].active) anyActive = true;

        if (!anyActive) {
            lives--;
            if (lives > 0) {
                resetBall();
            } else {
                // Game over
                if (score > highscore[currentLevel]) highscore[currentLevel] = score;
                if (score >= levels[currentLevel].entryScore && currentLevel + 1 > unlockedLevel && currentLevel < 4)
                    unlockedLevel = currentLevel + 1;
            state = ST_GAMEOVER; stopBGM();
        }
    }

    // Level complete
    if (allBricksDestroyed()) {
        if (score > highscore[currentLevel]) highscore[currentLevel] = score;
        if (currentLevel + 1 > unlockedLevel && currentLevel < 4)
            unlockedLevel = currentLevel + 1;
        playTone(1000, 500, 6);
        state = ST_GAMEOVER; stopBGM();
        }

        // Periodic lava drops from lava bricks
        static float lavaTimer = 0;
        lavaTimer += dt;
        while (lavaTimer > 0.5f) {
            lavaTimer -= 0.5f;
            for (int r = 0; r < GRID_ROWS; r++)
                for (int c = 0; c < GRID_COLS; c++)
                    if (bricks[r][c].alive && bricks[r][c].type == BT_LAVA) {
                        float bx,by,bw,bh; getBrickRect(r,c,bx,by,bw,bh);
                        spawnLavaDrop(bx, by);
                    }
        }

        // ─── RENDER ──────────────────────────────────────────────────────
        glClear(GL_COLOR_BUFFER_BIT);
        drawBackground(0.0f,0.0f,0.05f, 0.02f,0.0f,0.1f);

        updateParticles(dt);
        drawParticles();
        updateLavaDrops(dt);
        drawLavaDrops();

        // Draw bricks
        for (int r = 0; r < GRID_ROWS; r++)
            for (int c = 0; c < GRID_COLS; c++)
                drawBrick(r, c);

        drawPowerUps();
        drawPaddle();

        for (int i = 0; i < ballCount; i++)
            if (balls[i].active) drawBall(balls[i]);

        // HUD
        char buf[64];
        glColor3f(1.0f, 0.9f, 0.2f);
        snprintf(buf, sizeof(buf), "SCORE: %d", score);
        drawText(buf, 20, 20, 1.0f);
        glColor3f(0.3f, 0.8f, 1.0f);
        snprintf(buf, sizeof(buf), "LIVES: %d", lives);
        drawText(buf, WIN_W - 120, 20, 1.0f);
        glColor3f(1.0f, 0.6f, 0.2f);
        snprintf(buf, sizeof(buf), "LVL %d: %s", currentLevel+1, levels[currentLevel].name);
        drawText(buf, WIN_W/2 - 80, 20, 0.9f);
        if (combo > 0) {
            glColor3f(1.0f, 0.3f + 0.7f*sinf(combo*0.5f+glfwGetTime()*5), 0.3f);
            snprintf(buf, sizeof(buf), "COMBO x%d", combo);
            drawText(buf, WIN_W/2 - 60, WIN_H/2 + 100, 1.2f);
        }

        // Launch hint
        bool ballReady = false;
        for (int i = 0; i < ballCount; i++)
            if (balls[i].active && balls[i].vx == 0) ballReady = true;
        if (ballReady) {
            glColor3f(0.6f, 0.6f, 0.8f);
            drawText("CLICK or SPACE to launch", WIN_W/2 - 110, WIN_H/2 + 50, 1.0f);
        }

        if (state == ST_PAUSE) {
            glColor3f(1,1,1);
            drawText("PAUSED", WIN_W/2 - 60, WIN_H/2 - 20, 2.0f);
            drawText("ESC to resume", WIN_W/2 - 70, WIN_H/2 + 30, 1.0f);
        }

        glfwSwapBuffers(win);
        lastMouseL = mouseL;
        if (KEY_PRESSED(GLFW_KEY_P)) {
            if (state == ST_PLAY) state = ST_PAUSE;
            else if (state == ST_PAUSE) state = ST_PLAY;
        }

        // Save key states for edge detection
        lastKey[GLFW_KEY_1] = glfwGetKey(win, GLFW_KEY_1);
        lastKey[GLFW_KEY_2] = glfwGetKey(win, GLFW_KEY_2);
        lastKey[GLFW_KEY_3] = glfwGetKey(win, GLFW_KEY_3);
        lastKey[GLFW_KEY_4] = glfwGetKey(win, GLFW_KEY_4);
        lastKey[GLFW_KEY_5] = glfwGetKey(win, GLFW_KEY_5);
        lastKey[GLFW_KEY_SPACE] = glfwGetKey(win, GLFW_KEY_SPACE);
        lastKey[GLFW_KEY_ESCAPE] = glfwGetKey(win, GLFW_KEY_ESCAPE);
        lastKey[GLFW_KEY_P] = glfwGetKey(win, GLFW_KEY_P);
        lastKey[GLFW_KEY_F12] = glfwGetKey(win, GLFW_KEY_F12);
        lastKey[GLFW_KEY_LEFT] = glfwGetKey(win, GLFW_KEY_LEFT);
        lastKey[GLFW_KEY_RIGHT] = glfwGetKey(win, GLFW_KEY_RIGHT);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}