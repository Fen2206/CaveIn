#include "harinaga.h"
#include <GL/gl.h>
#include <cmath>
#include "input.h"
#include <X11/keysym.h>
#include "image.h"
#include "game.h"
enum Direction {
    DIR_S,
    DIR_SW,
    DIR_W,
    DIR_NW,
    DIR_N,
    DIR_NE,
    DIR_E,
    DIR_SE
};

static Direction dir = DIR_S;
static float px = 100.0f;
static float py = 100.0f;
static float speed = 4.0f;

static Image southFrames[4] = {
    Image("./assets/character_sprites/male_character/south/south_00.png"),
    Image("./assets/character_sprites/male_character/south/south_01.png"),
    Image("./assets/character_sprites/male_character/south/south_02.png"),
    Image("./assets/character_sprites/male_character/south/south_03.png")
};

static Image northFrames[4] = {
    Image("./assets/character_sprites/male_character/north/north_00.png"),
    Image("./assets/character_sprites/male_character/north/north_01.png"),
    Image("./assets/character_sprites/male_character/north/north_02.png"),
    Image("./assets/character_sprites/male_character/north/north_03.png")
};

static Image eastFrames[4] = {
    Image("./assets/character_sprites/male_character/east/east_00.png"),
    Image("./assets/character_sprites/male_character/east/east_01.png"),
    Image("./assets/character_sprites/male_character/east/east_02.png"),
    Image("./assets/character_sprites/male_character/east/east_03.png")
};

static Image westFrames[4] = {
    Image("./assets/character_sprites/male_character/west/west_00.png"),
    Image("./assets/character_sprites/male_character/west/west_01.png"),
    Image("./assets/character_sprites/male_character/west/west_02.png"),
    Image("./assets/character_sprites/male_character/west/west_03.png")
};

static Image neFrames[4] = {
    Image("./assets/character_sprites/male_character/northeast/northeast_00.png"),
    Image("./assets/character_sprites/male_character/northeast/northeast_01.png"),
    Image("./assets/character_sprites/male_character/northeast/northeast_02.png"),
    Image("./assets/character_sprites/male_character/northeast/northeast_03.png")
};

static Image nwFrames[4] = {
    Image("./assets/character_sprites/male_character/northwest/northwest_00.png"),
    Image("./assets/character_sprites/male_character/northwest/northwest_01.png"),
    Image("./assets/character_sprites/male_character/northwest/northwest_02.png"),
    Image("./assets/character_sprites/male_character/northwest/northwest_03.png")
};

static Image seFrames[4] = {
    Image("./assets/character_sprites/male_character/southeast/southeast_00.png"),
    Image("./assets/character_sprites/male_character/southeast/southeast_01.png"),
    Image("./assets/character_sprites/male_character/southeast/southeast_02.png"),
    Image("./assets/character_sprites/male_character/southeast/southeast_03.png")
};

static Image swFrames[4] = {
    Image("./assets/character_sprites/male_character/southwest/southwest_00.png"),
    Image("./assets/character_sprites/male_character/southwest/southwest_01.png"),
    Image("./assets/character_sprites/male_character/southwest/southwest_02.png"),
    Image("./assets/character_sprites/male_character/southwest/southwest_03.png")
};

static int currentFrame = 0;
static int frameCounter = 0;
void initPlayer()
{
    px = 100.0f;
    py = 100.0f;
    speed = 4.0f;

    for (int i = 0; i < 4; i++) {
        southFrames[i].init_gl();
        northFrames[i].init_gl();
        eastFrames[i].init_gl();
        westFrames[i].init_gl();
        neFrames[i].init_gl();
        nwFrames[i].init_gl();
        seFrames[i].init_gl();
        swFrames[i].init_gl();
    }
}
void updatePlayer()
{
    float dx = 0.0f;
    float dy = 0.0f;
    bool moving = false;

    if (g_keys[XK_a] || g_keys[XK_Left]) {
        dx -= 1.0f;
    }

    if (g_keys[XK_d] || g_keys[XK_Right]) {
        dx += 1.0f;
    }

    if (g_keys[XK_w] || g_keys[XK_Up]) {
        dy += 1.0f;
    }

    if (g_keys[XK_s] || g_keys[XK_Down]) {
        dy -= 1.0f;
    }

    if (dx != 0.0f || dy != 0.0f) {
        moving = true;

        // normalize diagonal movement
        float len = sqrt(dx*dx + dy*dy);
        dx /= len;
        dy /= len;

        px += dx * speed;
        py += dy * speed;

        // determine direction
        if (dx > 0 && dy > 0) dir = DIR_NE;
        else if (dx < 0 && dy > 0) dir = DIR_NW;
        else if (dx > 0 && dy < 0) dir = DIR_SE;
        else if (dx < 0 && dy < 0) dir = DIR_SW;
        else if (dx > 0) dir = DIR_E;
        else if (dx < 0) dir = DIR_W;
        else if (dy > 0) dir = DIR_N;
        else if (dy < 0) dir = DIR_S;
    }

    // animation
    if (moving) {
        frameCounter++;

        if (frameCounter > 8) {
            currentFrame++;

            if (currentFrame > 3) {
                currentFrame = 0;
            }

            frameCounter = 0;
        }
    } else {
        currentFrame = 0;
    }

    // clamp player inside screen
    if (px < 0.0f) {
        px = 0.0f;
    }

    if (px > g.xres) {
        px = g.xres;
    }

    if (py < 0.0f) {
        py = 0.0f;
    }

    if (py > g.yres) {
        py = g.yres;
    }
}

void drawPlayer()
{
    Image* frames;

    switch (dir) {
        case DIR_S:  frames = southFrames; break;
        case DIR_SW: frames = swFrames; break;
        case DIR_W:  frames = westFrames; break;
        case DIR_NW: frames = nwFrames; break;
        case DIR_N:  frames = northFrames; break;
        case DIR_NE: frames = neFrames; break;
        case DIR_E:  frames = eastFrames; break;
        case DIR_SE: frames = seFrames; break;
    }

    frames[currentFrame].show(25.0f, (int)px, (int)py, 0.0f);
}