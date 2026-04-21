#include "harinaga.h"
#include <GL/gl.h>
#include <cmath>
#include "input.h"
#include <X11/keysym.h>
#include "image.h"
#include "game.h"
#include "jgaribay.h"
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
float px = 100.0f;
float py = 100.0f;
static float speed = 4.0f;
static float sparkleTimer = 0.0f;
static float sparkleAngle = 0.0f;
static const float sparkleDuration = 0.6f;
static bool hurtActive = false;
static int hurtFrame = 0;
static int hurtFrameCounter = 0;
static const int hurtFrameCount = 6;
static const int hurtFrameDelay = 2;
static int hurtCooldown = 0;
static const int hurtCooldownDuration = 12;

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

static Image hurtSouthFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/south/south_00.png"),
    Image("./assets/character_sprites/male_character_hurt/south/south_01.png"),
    Image("./assets/character_sprites/male_character_hurt/south/south_02.png"),
    Image("./assets/character_sprites/male_character_hurt/south/south_03.png"),
    Image("./assets/character_sprites/male_character_hurt/south/south_04.png"),
    Image("./assets/character_sprites/male_character_hurt/south/south_05.png")
};

static Image hurtNorthFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/north/north_00.png"),
    Image("./assets/character_sprites/male_character_hurt/north/north_01.png"),
    Image("./assets/character_sprites/male_character_hurt/north/north_02.png"),
    Image("./assets/character_sprites/male_character_hurt/north/north_03.png"),
    Image("./assets/character_sprites/male_character_hurt/north/north_04.png"),
    Image("./assets/character_sprites/male_character_hurt/north/north_05.png")
};

static Image hurtEastFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/east/east_00.png"),
    Image("./assets/character_sprites/male_character_hurt/east/east_01.png"),
    Image("./assets/character_sprites/male_character_hurt/east/east_02.png"),
    Image("./assets/character_sprites/male_character_hurt/east/east_03.png"),
    Image("./assets/character_sprites/male_character_hurt/east/east_04.png"),
    Image("./assets/character_sprites/male_character_hurt/east/east_05.png")
};

static Image hurtWestFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/west/west_00.png"),
    Image("./assets/character_sprites/male_character_hurt/west/west_01.png"),
    Image("./assets/character_sprites/male_character_hurt/west/west_02.png"),
    Image("./assets/character_sprites/male_character_hurt/west/west_03.png"),
    Image("./assets/character_sprites/male_character_hurt/west/west_04.png"),
    Image("./assets/character_sprites/male_character_hurt/west/west_05.png")
};

static Image hurtNeFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_00.png"),
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_01.png"),
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_02.png"),
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_03.png"),
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_04.png"),
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_05.png")
};

static Image hurtNwFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_00.png"),
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_01.png"),
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_02.png"),
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_03.png"),
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_04.png"),
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_05.png")
};

static Image hurtSeFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_00.png"),
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_01.png"),
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_02.png"),
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_03.png"),
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_04.png"),
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_05.png")
};

static Image hurtSwFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_00.png"),
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_01.png"),
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_02.png"),
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_03.png"),
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_04.png"),
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_05.png")
};

static int currentFrame = 0;
static int frameCounter = 0;

void triggerPlayerSparkle()
{
    sparkleTimer = sparkleDuration;
    sparkleAngle = 0.0f;
}

void triggerPlayerHurt()
{
    if (hurtActive || hurtCooldown > 0) {
        return;
    }
    hurtActive = true;
    hurtSound.play();
    hurtFrame = 0;
    hurtFrameCounter = 0;
    hurtCooldown = hurtCooldownDuration;
}
void initPlayer()
{
    px = g.xres * 0.5f;
    py = 120.0f;
    speed = 4.0f;
    sparkleTimer = 0.0f;
    sparkleAngle = 0.0f;
    hurtActive = false;
    hurtFrame = 0;
    hurtFrameCounter = 0;
    hurtCooldown = 0;

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
    // hurt frames
    for (int i = 0; i < hurtFrameCount; i++) {
        hurtSouthFrames[i].init_gl();
        hurtNorthFrames[i].init_gl();
        hurtEastFrames[i].init_gl();
        hurtWestFrames[i].init_gl();
        hurtNeFrames[i].init_gl();
        hurtNwFrames[i].init_gl();
        hurtSeFrames[i].init_gl();
        hurtSwFrames[i].init_gl();
    }
}

void updatePlayer()
{
    //sparkle timer update
    if (sparkleTimer > 0.0f) {
        sparkleTimer -= 1.0f / 60.0f;
        if (sparkleTimer < 0.0f) {
            sparkleTimer = 0.0f;
        }
        sparkleAngle += 8.0f;
    }
    //hurt timer update
    if (hurtActive) {
        hurtFrameCounter++;
        if (hurtFrameCounter >= hurtFrameDelay) {
            hurtFrameCounter = 0;
            hurtFrame++;
            if (hurtFrame >= hurtFrameCount) {
                hurtFrame = 0;
                hurtActive = false;
            }
        }
    }
    if (hurtCooldown > 0) {
        hurtCooldown--;
    }

    float dx = 0.0f;
    float dy = 0.0f;
    bool moving = false;
    // input handling, if not hurt, allow movement input, if hurt, ignore input and play hurt animation
    if (!hurtActive) {
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

            float len = sqrt(dx*dx + dy*dy);
            dx /= len;
            dy /= len;

            px += dx * speed;
            py += dy * speed;

            if (dx > 0 && dy > 0) dir = DIR_NE;
            else if (dx < 0 && dy > 0) dir = DIR_NW;
            else if (dx > 0 && dy < 0) dir = DIR_SE;
            else if (dx < 0 && dy < 0) dir = DIR_SW;
            else if (dx > 0) dir = DIR_E;
            else if (dx < 0) dir = DIR_W;
            else if (dy > 0) dir = DIR_N;
            else if (dy < 0) dir = DIR_S;
        }
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
    const float center = g.xres * 0.5f;
    const float horizontalLimit = 140.0f;

    if (px < center - horizontalLimit) {
        px = center - horizontalLimit;
    }

    if (px > center + horizontalLimit) {
        px = center + horizontalLimit;
    }

    if (py < 0.0f) {
        py = 0.0f;
    }
}

void drawPlayer()
{
    Image* frames;
    Image* hurtFrames;
    // select normal and hurt frames based on direction
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

    switch (dir) {
        case DIR_S:  hurtFrames = hurtSouthFrames; break;
        case DIR_SW: hurtFrames = hurtSwFrames; break;
        case DIR_W:  hurtFrames = hurtWestFrames; break;
        case DIR_NW: hurtFrames = hurtNwFrames; break;
        case DIR_N:  hurtFrames = hurtNorthFrames; break;
        case DIR_NE: hurtFrames = hurtNeFrames; break;
        case DIR_E:  hurtFrames = hurtEastFrames; break;
        case DIR_SE: hurtFrames = hurtSeFrames; break;
    }

    float sx = px - g.cameraX;
    float sy = py - g.cameraY;

    if (hurtActive) {
        hurtFrames[hurtFrame].show(25.0f, (int)sx, (int)sy, 0.0f);
    } else {
        frames[currentFrame].show(25.0f, (int)sx, (int)sy, 0.0f);
    }

    if (sparkleTimer > 0.0f) {
        const float progress = sparkleTimer / sparkleDuration;
        const float orbitRadius = 22.0f + (1.0f - progress) * 8.0f;
        const float sparkleSize = 5.0f + sinf(sparkleAngle * 0.08f) * 1.5f;

        for (int i = 0; i < 4; i++) {
            const float angle = sparkleAngle * 0.04f + i * 1.57079632679f;
            const float sparkleX = sx + cosf(angle) * orbitRadius;
            const float sparkleY = sy + sinf(angle) * orbitRadius;
            g.diamond.show(
                sparkleSize,
                (int)sparkleX,
                (int)sparkleY,
                sparkleAngle + i * 45.0f,
                0);
        }
    }
}
