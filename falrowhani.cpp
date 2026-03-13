// falrowhani.cpp

#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <cstdio>

#include "falrowhani.h"
#include "image.h"
#include "game.h"     
#include "harinaga.h"

extern Global g;
extern float px;
extern float py;


void initPowerups() {}
void updatePowerups() {}
void drawPowerups() {}

void initGems() {}
void updateGems() {}
void drawGems() {}

//for rock animation in title screen
static const int   MAX_ROCKS = 500;
static const float FLOOR_Y  = 210.0f;

static Image rock("./assets/rocks.png");

struct Rock {
    float x, y;
    float vx, vy;
    float size;
    float rot, rotSpeed;
    bool  alive;
    bool  settled;
};

static Rock rocks[MAX_ROCKS];
static int gx = 0, gy = 0;
static int initialized = 0;

static inline bool AABB(float ax, float ay, float aw, float ah,
                        float bx, float by, float bw, float bh)
{
    bool collisionX = ax + aw >= bx && bx + bw >= ax;
    bool collisionY = ay + ah >= by && by + bh >= ay;
    return collisionX && collisionY;
}

static void spawnRock(int i)
{
    rocks[i].alive   = true;
    rocks[i].settled = false;

    rocks[i].x = 180.0f + frand01() * 140.0f;
    rocks[i].y = 580.0f - frand01() * 120.0f;

    rocks[i].vx = (frand01() - 0.5f) * 0.6f;
    rocks[i].vy = -(1.5f + frand01() * 2.5f);

    rocks[i].size = 25.0f + frand01() * 23.0f;

    rocks[i].rot      = frand01() * 360.0f;
    rocks[i].rotSpeed = (frand01() - 0.5f) * 4.0f;
}

void titleAnimationInit(int xres, int yres)
{
    gx = xres;
    gy = yres;
    initialized = 1;

    for (int i = 0; i < MAX_ROCKS; i++) {
        rocks[i].alive = false;
        rocks[i].settled = false;
    }

    rock.init_gl();
}

void titleAnimationUpdate(float gravity)
{
    if (!initialized) return;

    // Spawn new rocks
    static int spawnCounter = 0;
    spawnCounter++;
    if (spawnCounter >= 5) {
        spawnCounter = 0;
        for (int i = 0; i < MAX_ROCKS; i++) {
            if (!rocks[i].alive) {
                spawnRock(i);
                break;
            }
        }
    }

    // Update physics
    for (int i = 0; i < MAX_ROCKS; i++) {
        if (!rocks[i].alive) continue;

        rocks[i].vy += gravity * 0.15f;

        rocks[i].x += rocks[i].vx;
        rocks[i].y += rocks[i].vy;
        rocks[i].rot += rocks[i].rotSpeed;

        // Floor collision: stop immediately
        if (rocks[i].y - rocks[i].size <= FLOOR_Y) {
            rocks[i].y = FLOOR_Y + rocks[i].size;
            rocks[i].vx = 0.0f;
            rocks[i].vy = 0.0f;
            rocks[i].rotSpeed = 0.0f;
        }
    }
}

void titleAnimationRender()
{
    if (!initialized) return;

    for (int i = 0; i < MAX_ROCKS; i++) {
        if (!rocks[i].alive) continue;

        rock.show(rocks[i].size,
                  (int)rocks[i].x,
                  (int)rocks[i].y,
                  rocks[i].rot,
                  0);
    }
}

//drawing properties (prop) in game state

struct Prop {
    float x, y;
    int type;
    bool active;
};

enum {
    PROP_DIAMOND = 0,
    PROP_SPIKE   = 1,
    
};

#define MAX_PROPS 3000
static Prop props[MAX_PROPS];
static int propCount = 0;

static const float CHUNK_SIZE = 256.0f;
static const float MIN_DIST   = 36.0f;
static const int   TARGET_PER_CHUNK = 18;

// to be continuted 
static float cameraX = 0.0f;
static float cameraY = 0.0f;


static void addProp(float x, float y, int type)
{
    if (propCount >= MAX_PROPS) return;
    props[propCount].x = x; //update x 
    props[propCount].y = y; //update y 
    props[propCount].type = type; //draw type
    props[propCount].active = true;
    propCount++;
}

void propsGenerateInitial()
{
    propCount = 0;

    //setting these values will affect how many props are generated and how they are spaced out
    const int   TOTAL_PROPS   = 20;     // lower = fewer items
    const float MIN_DIST      = 70.0f;  // higher = more spaced out
    const float CORRIDOR_HALF = 70.0f;  // half-width so properties can be placed in the middle 
    const float TOP_MARGIN    = 70.0f;
    const float BOT_MARGIN    = 70.0f;


    const float min2 = MIN_DIST * MIN_DIST;

    float cx = g.xres * 0.5f;

    int attempts = TOTAL_PROPS * 80;
    for (int k = 0; k < attempts && propCount < TOTAL_PROPS; k++) {

        float x = (cx - CORRIDOR_HALF) + frand01() * (CORRIDOR_HALF * 2.0f);
        float y = TOP_MARGIN + frand01() * (g.yres - TOP_MARGIN - BOT_MARGIN);

        // spacing check
        int ok = 1;
        for (int i = 0; i < propCount; i++) {
            float dx = props[i].x - x;
            float dy = props[i].y - y;
            if (dx*dx + dy*dy < min2) { ok = 0; break; }
        }
        if (!ok) continue;

        // 60% diamonds, 40% spikes
        float r = frand01();
        int type = (r < 0.60f) ? PROP_DIAMOND : PROP_SPIKE;

        addProp(x, y, type);
    }
}


void propsRender()
{
    const float diamondSize = 24.0f;
    const float spikeSize   = 28.0f;

    for (int i = 0; i < propCount; i++) {
        if (!props[i].active) continue;
        float sx = props[i].x - cameraX;
        float sy = props[i].y - cameraY;


        if (props[i].type == PROP_DIAMOND) {
            g.diamond.show(diamondSize, (int)sx, (int)sy, 0.0f, 0);
        } else if (props[i].type == PROP_SPIKE) {
            g.spike.show(spikeSize, (int)sx, (int)sy, 0.0f, 0);
        }
    }
}

void propsCheckCollisionsWithPlayer()
{
    const float playerW = 32.0f;
    const float playerH = 32.0f;

    const float diamondSize = 24.0f;
    const float spikeSize   = 28.0f;

    //  checkign corners
    float pLeft = px - playerW * 0.5f;
    float pBot  = py - playerH * 0.5f;

    for (int i = 0; i < propCount; i++) {
        if (!props[i].active) continue;

        float sz;

        if (props[i].type == PROP_DIAMOND)
            sz = diamondSize;
        else
            sz = spikeSize;

       //checking corners 
        float dLeft = props[i].x - sz * 0.5f;
        float dBot  = props[i].y - sz * 0.5f;

        if (AABB(pLeft, pBot, playerW, playerH, dLeft, dBot, sz, sz)) {
            if (props[i].type == PROP_DIAMOND) {
              printf("diamond collected");
                props[i].active = false;
            } else {
              printf("Hit spike!");
            }
        }
    }
}