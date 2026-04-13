#ifndef FALROWHANI_H
#define FALROWHANI_H

#include "image.h"
#include <cstdlib>


static inline float frand01() {
    return (float)rand() / (float)RAND_MAX;
}

void titleAnimationInit(int xres, int yres);
void titleAnimationUpdate(float gravity);
void titleAnimationRender();
//static void settleRock(int i, float newY);


void initPowerups();
void updatePowerups();
void drawPowerups();


void initGems();
void updateGems();
void drawGems();


void propsGenerateInitial();
void propsRender();
void propsCheckCollisionsWithPlayer();
void propsUpdate();
void gamePhysics();
void propsUpdateStreaming();


#endif