#include "game.h"
#include "harinaga.h"
#include "jgaribay.h"
#include "falrowhani.h"

int score = 0;
bool gameOver = false;

void checkCollisions();

void initGame()
{
    score = 0;
    gameOver = false;

    initPlayer();      // harinaga.cpp
    initObstacles();   // jgaribay.cpp
    initPowerups();    // falrowhani.cpp
    initGems();        // falrowhani.cpp
}

void gamePhysics()
{
    updatePlayer();
    updateObstacles();
    updatePowerups();
    updateGems();

    checkCollisions();
}

void renderGame()
{
    drawObstacles();
    drawGems();
    drawPowerups();
    drawPlayer();
}

void checkCollisions()
{
    
}