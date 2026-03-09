#include "game.h"
#include "harinaga.h"
#include "jgaribay.h"
#include "falrowhani.h"
#include <cstring>

Global::Global()
    : background("./assets/new.png"),
      game("./assets/cave2.png"),
      diamond("./assets/dia.png"),
      spike("./assets/spikes.png")
{
    xres = 500;
    yres = 650;
    memset(keys, 0, 65536);
    mouse_cursor_on = 1;
    state = STATE_TITLE;
    menuSelection = 0;
}

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
   // g.gameBackground.show(g.xres/2, g.xres/2, g.yres/2, 0.0f);
    drawObstacles();
    drawGems();
    drawPowerups();
    drawPlayer();
}

void checkCollisions()
{
    
}