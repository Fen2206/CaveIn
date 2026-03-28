#include "game.h"
#include "harinaga.h"
#include "jgaribay.h"
#include "falrowhani.h"
#include <cstring>

Global::Global()
    : background("./assets/new.png"),
      game("./assets/t.png"),
      diamond("./assets/dia.png"),
      spike("./assets/spikes.png"),
    health5("./assets/h1.png"),
    health4("./assets/h6.png"),
    health3("./assets/h7.png"),
    health2("./assets/h3.png"),
    health1("./assets/h4.png"),
    health0("./assets/h5.png"),
    fireRock("./assets/fire.png"),
    fireImpact("./assets/bigger.png")

      
    {
    xres = 500;
    yres = 570;
    memset(keys, 0, 65536);
    mouse_cursor_on = 1;
    state = STATE_TITLE;
    menuSelection = 0;


    cameraX = 0.0f;
    cameraY = 0.0f;
    propsGenerateInitial();
   

    maxHealth = 5;
    health = 5;
    score = 0;
    hurtTimer = 0;
}

int score = 0;
bool gameOver = false;

void checkCollisions();

void initGame()
{
    score = 0;
    gameOver = false;

    g.health = g.maxHealth;
    g.score = 0;
    g.hurtTimer = 0;

    initPlayer();      // harinaga.cpp
    initObstacles();   // jgaribay.cpp
    initPowerups();    // falrowhani.cpp
    initGems();        // falrowhani.cpp
}
/*
void gamePhysics()
{
    updatePlayer();
    updateObstacles();
    updatePowerups();
    updateGems();

    checkCollisions();
}
*/
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