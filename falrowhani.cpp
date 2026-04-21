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

static const int SPIKE_HURT_COOLDOWN = 12;
static const int FIRE_ROCK_HURT_COOLDOWN = 30;

void initPowerups() {}
void updatePowerups() {}
void drawPowerups() {}

void initGems() {}
void updateGems() {}
void drawGems() {}

// --------------------------------------------------------
// Title screen falling rocks
// --------------------------------------------------------
static const int   MAX_ROCKS = 500;
static const float FLOOR_Y   = 210.0f;

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

// --------------------------------------------------------
// Gameplay props
// diamonds, spikes, fire rocks
// --------------------------------------------------------

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
static const float FIRE_ROCK_SIZE   = 30.0f;
static const float FIRE_IMPACT_SIZE = 40.0f;

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
        props[propCount].vx = (frand01() - 0.5f) * 0.8f;
        props[propCount].vy = 3.5f + frand01() * 2.5f;
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
            props[i].vy += 0.15f;
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
    const float diamondSize = 24.0f;
    const float spikeSize   = 28.0f;

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
    const float spikeSize   = 28.0f;

    float pLeft = px - playerW * 0.5f;
    float pBot  = py - playerH * 0.5f;

    for (int i = 0; i < propCount; i++) {
        if (!props[i].active)
            continue;

        float sz = 0.0f;

        if (props[i].type == PROP_DIAMOND)
            sz = diamondSize;
        else if (props[i].type == PROP_SPIKE)
            sz = spikeSize;
        else if (props[i].type == PROP_FIRE_ROCK)
            sz = props[i].landed ? FIRE_IMPACT_SIZE : FIRE_ROCK_SIZE;

        float dLeft = props[i].x - sz * 0.5f;
        float dBot  = props[i].y - sz * 0.5f;

        if (AABB(pLeft, pBot, playerW, playerH, dLeft, dBot, sz, sz)) {
            if (props[i].type == PROP_DIAMOND) {
                props[i].active = false;
                g.score += 10;
                triggerPlayerSparkle();
                playSound(GEM_SPARKLE);
            }
            else if (props[i].type == PROP_SPIKE) {
                if (g.hurtTimer <= 0) {
                    g.health--;
                    if (g.health < 0)
                        g.health = 0;
                    g.hurtTimer = SPIKE_HURT_COOLDOWN;
                    triggerPlayerHurt();
                    playSound(PLAYER_HURT);
                }
            }
            else if (props[i].type == PROP_FIRE_ROCK) {
                if (g.hurtTimer <= 0) {
                    g.health -= 2;
                    if (g.health < 0)
                        g.health = 0;
                    g.hurtTimer = FIRE_ROCK_HURT_COOLDOWN;
                    triggerPlayerHurt();
                    playSound(PLAYER_HURT);
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
    updateObstacles();
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

    propsUpdateStreaming();
    propsCheckCollisionsWithPlayer();
    updateLevelTimer();
}
