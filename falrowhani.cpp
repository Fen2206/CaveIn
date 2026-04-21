// falrowhani.cpp

#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <cstdio>

#include "falrowhani.h"
#include "image.h"
#include "game.h"
#include "harinaga.h"
#include "jgaribay.h"

extern Global g;
extern float px;
extern float py;

static const int SPIKE_HURT_COOLDOWN = 60;
static const int FIRE_ROCK_HURT_COOLDOWN = 45;

void initPowerups() {}
void updatePowerups() {}
void drawPowerups() {}

void initGems() {}
void updateGems() {}
void drawGems() {}

static inline bool AABB(float ax, float ay, float aw, float ah,
                        float bx, float by, float bw, float bh)
{
    bool collisionX = ax + aw >= bx && bx + bw >= ax;
    bool collisionY = ay + ah >= by && by + bh >= ay;
    return collisionX && collisionY;
}

static const int   MAX_ROCKS = 50;
static const float FLOOR_Y   = 130.0f;

static Image caveTitle("./assets/cave.png");
static Image inTitle("./assets/in.png");

static float caveX, caveY;
static float caveTargetX, caveTargetY;
static float caveW = 320.0f;

static float inX, inY;
static float inTargetX, inTargetY;
static float inW = 120.0f;

static bool caveArrived = false;
static bool inArrived   = false;

static Image rock("./assets/rocks.png");

struct Rock {
    float x, y;
    float vx, vy;
    float size;
    float rot, rotSpeed;
    bool alive;
    bool settled;
};

static Rock rocks[MAX_ROCKS];
static int gx = 0, gy = 0;
static int initialized = 0;


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
    caveTitle.init_gl();
    inTitle.init_gl();

    // Final resting position of CAVE
    caveTargetX = gx / 2.0f;
    caveTargetY = gy / 2.0f + 120.0f;

    // Final resting position of IN, centered below CAVE
    inTargetX = gx / 2.0f;
    inTargetY = caveTargetY - 85.0f;

    // Start offscreen
    caveX = gx + caveW;
    caveY = caveTargetY;

    inX = -inW;
    inY = inTargetY;

    caveArrived = false;
    inArrived = false;
}

void titleAnimationUpdate(float gravity)
{
    if (!initialized) return;

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

    for (int i = 0; i < MAX_ROCKS; i++) {
        if (!rocks[i].alive) continue;

        rocks[i].vy += gravity * 0.15f;
        rocks[i].x += rocks[i].vx;
        rocks[i].y += rocks[i].vy;
        rocks[i].rot += rocks[i].rotSpeed;

        if (rocks[i].y - rocks[i].size <= FLOOR_Y) {
            rocks[i].y = FLOOR_Y + rocks[i].size;
            rocks[i].vx = 0.0f;
            rocks[i].vy = 0.0f;
            rocks[i].rotSpeed = 0.0f;
            rocks[i].settled = true;
        }
    }

    // Move CAVE in from the right first
    if (!caveArrived) {
        caveX -= 12.0f;
        if (caveX <= caveTargetX) {
            caveX = caveTargetX;
            caveArrived = true;
        }
    }
  // move IN from the left
    else if (!inArrived) {
        inX += 12.0f;
        if (inX >= inTargetX) {
            inX = inTargetX;
            inArrived = true;
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

    caveTitle.show((int)caveW, (int)caveX, (int)caveY, 0.0f, 0);
    inTitle.show((int)inW, (int)inX, (int)inY, 0.0f, 0);
}

// Gameplay props
// diamonds, spikes, fire rocks

struct Prop {
    float x, y;
    float vx, vy;
    int type;
    bool active;
    bool landed;
};

enum {
    PROP_DIAMOND   = 0,
    PROP_SPIKE     = 1,
    PROP_FIRE_ROCK = 2
};

#define MAX_PROPS 3000
static Prop props[MAX_PROPS];
static int propCount = 0;

static const float CHUNK_SIZE = 256.0f;
static const float MIN_DIST = 70.0f;
static const int   TARGET_PER_CHUNK = 6;

static int highestChunkGenerated = -1;

static int fireRockSpawnTimer = 0;
static const float FIRE_ROCK_SIZE   = 22.0f;
static const float FIRE_IMPACT_SIZE = 25.0f;
static const float FIRE_ROCK_DRIFT_SPEED = 0.45f;
static const float FIRE_ROCK_MIN_FALL_SPEED = 1.8f;
static const float FIRE_ROCK_FALL_SPEED_RANGE = 1.2f;
static const float FIRE_ROCK_GRAVITY = 0.07f;

static void addProp(float x, float y, int type)
{
    if (propCount >= MAX_PROPS) return;

    props[propCount].x = x;
    props[propCount].y = y;
    props[propCount].vx = 0.0f;
    props[propCount].vy = 0.0f;
    props[propCount].type = type;
    props[propCount].active = true;
    props[propCount].landed = false;

    if (type == PROP_FIRE_ROCK) {
        props[propCount].vx = (frand01() - 0.5f) * FIRE_ROCK_DRIFT_SPEED;
        props[propCount].vy = FIRE_ROCK_MIN_FALL_SPEED +
                              frand01() * FIRE_ROCK_FALL_SPEED_RANGE;
    }

    propCount++;
}

static void removeOldProps()
{
    float keepBelowY = g.cameraY - (g.yres * 1.5f);
    int write = 0;

    for (int read = 0; read < propCount; read++) {
        if (!props[read].active)
            continue;

        if (props[read].y < keepBelowY)
            continue;

        if (write != read)
            props[write] = props[read];

        write++;
    }

    propCount = write;
}

static void generateChunk(int chunkIndex)
{
    const float center = g.xres * 0.5f;
    const float halfWidth = 70.0f;
    const float topPad = 20.0f;
    const float bottomPad = 20.0f;
    const float min2 = MIN_DIST * MIN_DIST;

    float yStart = chunkIndex * CHUNK_SIZE + topPad;
    float yEnd   = (chunkIndex + 1) * CHUNK_SIZE - bottomPad;

    int added = 0;
    int attempts = TARGET_PER_CHUNK * 40;

    for (int k = 0; k < attempts && added < TARGET_PER_CHUNK; k++) {
        float x = (center - halfWidth) + frand01() * (halfWidth * 2.0f);
        float y = yStart + frand01() * (yEnd - yStart);

        int ok = 1;
        for (int i = 0; i < propCount; i++) {
            if (!props[i].active)
                continue;

            if (props[i].type == PROP_FIRE_ROCK)
                continue;

            float dx = props[i].x - x;
            float dy = props[i].y - y;
            if (dx * dx + dy * dy < min2) {
                ok = 0;
                break;
            }
        }

        if (!ok)
            continue;

        int type = (frand01() < 0.60f) ? PROP_DIAMOND : PROP_SPIKE;
        addProp(x, y, type);
        added++;
    }
}

static void spawnFireRockFromSky()
{
    float center = g.xres * 0.5f;
    float halfWidth = 90.0f;

    float x = (center - halfWidth) + frand01() * (halfWidth * 2.0f);

    // spawn above visible area
    float y = g.cameraY + g.yres + 120.0f;

    addProp(x, y, PROP_FIRE_ROCK);
}

void propsGenerateInitial()
{
    propCount = 0;
    highestChunkGenerated = -1;
    fireRockSpawnTimer = 0;

    int initialTopChunk = (int)((g.yres * 2.0f) / CHUNK_SIZE);

    for (int chunk = 0; chunk <= initialTopChunk; chunk++) {
        generateChunk(chunk);
        highestChunkGenerated = chunk;
    }
}

void propsUpdateStreaming()
{
    removeOldProps();

    float wantedTopY = g.cameraY + (g.yres * 2.0f);
    int wantedChunk = (int)(wantedTopY / CHUNK_SIZE);

    while (highestChunkGenerated < wantedChunk) {
        highestChunkGenerated++;
        generateChunk(highestChunkGenerated);
    }

    // spawn fire rocks
    fireRockSpawnTimer++;
    if (fireRockSpawnTimer >= 45) {
        fireRockSpawnTimer = 0;
        if (!g.debugMode)
            spawnFireRockFromSky();
    }

    // update fire rocks
    for (int i = 0; i < propCount; i++) {
        if (!props[i].active)
            continue;

        if (props[i].type != PROP_FIRE_ROCK)
            continue;

        if (!props[i].landed) {
            // In your game, smaller y is lower, so subtract vy to fall
            props[i].vy += FIRE_ROCK_GRAVITY;
            props[i].x += props[i].vx;
            props[i].y -= props[i].vy;

            // land near lower part of visible screen
            float impactY = g.cameraY + 100.0f;

            if (props[i].y <= impactY) {
                props[i].y = impactY;
                props[i].vx = 0.0f;
                props[i].vy = 0.0f;
                props[i].landed = true;
            }
        } else {
            // keep impact image in place until camera moves away
        }
    }
}

void propsRender()
{
    const float diamondSize = 16.0f;
    const float spikeSize   = 20.0f;

    for (int i = 0; i < propCount; i++) {
        if (!props[i].active)
            continue;

        float sx = props[i].x - g.cameraX;
        float sy = props[i].y - g.cameraY;

        if (props[i].type == PROP_DIAMOND) {
            g.diamond.show(diamondSize, (int)sx, (int)sy, 0.0f, 0);
        }
        else if (props[i].type == PROP_SPIKE) {
            g.spike.show(spikeSize, (int)sx, (int)sy, 0.0f, 0);
        }
        else if (props[i].type == PROP_FIRE_ROCK) {
            if (!props[i].landed) {
                g.fireRock.show(FIRE_ROCK_SIZE, (int)sx, (int)sy, 0.0f, 0);
            } else {
                g.fireImpact.show(FIRE_IMPACT_SIZE, (int)sx, (int)sy, 0.0f, 0);
            }
        }
    }
}

void propsCheckCollisionsWithPlayer()
{
    const float playerW = 32.0f;
    const float playerH = 32.0f;

    const float diamondSize = 24.0f;
    const float spikeHitboxW = 18.0f;
    const float spikeHitboxH = 16.0f;

    float pLeft = px - playerW * 0.5f;
    float pBot  = py - playerH * 0.5f;

    for (int i = 0; i < propCount; i++) {
        if (!props[i].active)
            continue;

        float hitW = 0.0f;
        float hitH = 0.0f;

        if (props[i].type == PROP_DIAMOND) {
            hitW = diamondSize;
            hitH = diamondSize;
        }
        else if (props[i].type == PROP_SPIKE) {
            hitW = spikeHitboxW;
            hitH = spikeHitboxH;
        }
        else if (props[i].type == PROP_FIRE_ROCK) {
            hitW = props[i].landed ? FIRE_IMPACT_SIZE : FIRE_ROCK_SIZE;
            hitH = hitW;
        }

        float dLeft = props[i].x - hitW * 0.5f;
        float dBot  = props[i].y - hitH * 0.5f;

        if (AABB(pLeft, pBot, playerW, playerH, dLeft, dBot, hitW, hitH)) {
            if (props[i].type == PROP_DIAMOND) {
                props[i].active = false;
                g.score += 10;
                triggerPlayerSparkle();
                gemSound.play();
            }
            else if (props[i].type == PROP_SPIKE) {
                if (g.hurtTimer <= 0 && g.shieldTimer <= 0) {
                    g.health--;
                    if (g.health < 0)
                        g.health = 0;
                    g.hurtTimer = SPIKE_HURT_COOLDOWN;
                    triggerPlayerHurt();
                    hurtSound.play();
                }
            }
            else if (props[i].type == PROP_FIRE_ROCK) {
                if (g.hurtTimer <= 0) {
                    g.health -= 1;
                    if (g.health < 0)
                        g.health = 0;
                    g.hurtTimer = FIRE_ROCK_HURT_COOLDOWN;
                    triggerPlayerHurt();
                    hurtSound.play();
                }
            }
        }
    }
}

void gamePhysics()
{
    if (isLevelPassed()) {
        return;
    }

    updatePlayer();
    updatePowerups();
    updateGems();

    g.cameraX = 0.0f;
    g.cameraY = py - (g.yres * 0.5f);

    if (g.cameraY < 0.0f) {
        g.cameraY = 0.0f;
    }

    if (g.hurtTimer > 0) {
        g.hurtTimer--;
    }
    if (g.shieldTimer > 0) {
        g.shieldTimer--;
    }

    propsUpdateStreaming();
    propsCheckCollisionsWithPlayer();
    updateLevelTimer();
}
