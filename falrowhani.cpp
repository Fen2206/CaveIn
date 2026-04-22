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
    float impactX, impactY;
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
static const float FIRE_ROCK_WARNING_DISTANCE = 220.0f;

static int getDifficultyStep()
{
    int step = g.level - 1;
    if (step < 0)
        step = 0;
    if (step > 4)
        step = 4;
    return step;
}

static int getFireRockSpawnInterval()
{
    int interval = 100 - getDifficultyStep() * 15;
    if (interval < 45)
        interval = 45;
    return interval;
}

static float getFireRockDriftSpeed()
{
    return 0.25f + getDifficultyStep() * 0.05f;
}

static float getFireRockMinFallSpeed()
{
    return 1.45f + getDifficultyStep() * 0.25f;
}

static float getFireRockFallSpeedRange()
{
    return 0.95f + getDifficultyStep() * 0.15f;
}

static float getFireRockGravity()
{
    return 0.055f + getDifficultyStep() * 0.01f;
}

static float getSpikeChance()
{
    return 0.25f + getDifficultyStep() * 0.05f;
}

static int getFireRockDamage()
{
    return (g.level >= 4) ? 2 : 1;
}

static void drawFireRockShadow(float x, float y, float impactY)
{
    float distance = y - impactY;
    if (distance < 0.0f)
        distance = 0.0f;
    if (distance > FIRE_ROCK_WARNING_DISTANCE)
        distance = FIRE_ROCK_WARNING_DISTANCE;

    float warningProgress = 1.0f - (distance / FIRE_ROCK_WARNING_DISTANCE);
    float radiusX = 11.0f + warningProgress * 12.0f;
    float radiusY = 4.0f + warningProgress * 5.0f;
    float alpha = 0.18f + warningProgress * 0.32f;
    float sx = x - g.cameraX;
    float sy = impactY - g.cameraY;

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, alpha);

    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(sx, sy);
        for (int i = 0; i <= 32; i++) {
            float angle = i * 6.28318530718f / 32.0f;
            glVertex2f(sx + cosf(angle) * radiusX,
                       sy + sinf(angle) * radiusY);
        }
    glEnd();

    glColor4ub(255, 255, 255, 255);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

static void addProp(float x, float y, int type)
{
    if (propCount >= MAX_PROPS) return;

    props[propCount].x = x;
    props[propCount].y = y;
    props[propCount].vx = 0.0f;
    props[propCount].vy = 0.0f;
    props[propCount].impactX = x;
    props[propCount].impactY = y;
    props[propCount].type = type;
    props[propCount].active = true;
    props[propCount].landed = false;

    if (type == PROP_FIRE_ROCK) {
        props[propCount].vx = (frand01() - 0.5f) * getFireRockDriftSpeed();
        props[propCount].vy = getFireRockMinFallSpeed() +
                              frand01() * getFireRockFallSpeedRange();
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

        int type = (frand01() < getSpikeChance()) ? PROP_SPIKE : PROP_DIAMOND;
        addProp(x, y, type);
        added++;
    }
}

static void spawnFireRockFromSky()
{
    float center = g.xres * 0.5f;
    float horizontalLimit = 140.0f;
    float targetRadius = 35.0f;

    float targetX = px + (frand01() - 0.5f) * (targetRadius * 2.0f);
    if (targetX < center - horizontalLimit)
        targetX = center - horizontalLimit;
    if (targetX > center + horizontalLimit)
        targetX = center + horizontalLimit;

    float targetY = py + (frand01() - 0.5f) * (targetRadius * 2.0f);
    if (targetY < 0.0f)
        targetY = 0.0f;

    // spawn above visible area
    float y = g.cameraY + g.yres + 120.0f;

    addProp(targetX, y, PROP_FIRE_ROCK);
    props[propCount - 1].impactX = targetX;
    props[propCount - 1].impactY = targetY;
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
    if (fireRockSpawnTimer >= getFireRockSpawnInterval()) {
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
            props[i].vy += getFireRockGravity();
            props[i].x += props[i].vx;
            props[i].y -= props[i].vy;

            if (props[i].y <= props[i].impactY) {
                props[i].x = props[i].impactX;
                props[i].y = props[i].impactY;
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
                drawFireRockShadow(props[i].impactX, props[i].y, props[i].impactY);
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
                if (g.hurtTimer <= 0 && g.shieldTimer <= 0) {
                    g.health -= getFireRockDamage();
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
    updateGems();
    updatePowerups();

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
    if (g.speedTimer > 0) {
        g.speedTimer--;
	speed = 7.0f;
    } else {
	speed = 4.0f;
    }

    propsUpdateStreaming();
    propsCheckCollisionsWithPlayer();
    updateLevelTimer();
}
