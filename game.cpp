#include "game.h"
#include "harinaga.h"
#include "jgaribay.h"
#include "falrowhani.h"
#include "fonts.h"
#include <cstring>
#include <cstdio>

Global::Global()
    : background("./assets/new.png"),
      game("./assets/t.png"),
      diamond("./assets/dia.png"),
      spike("./assets/spikes.png"),
    health8("./assets/h1.png"),
    health7("./assets/h6.png"),
    health6("./assets/h7.png"),
    health5("./assets/h8.png"),
    health4("./assets/h9.png"),
    health3("./assets/h3.png"),
    health2("./assets/h4.png"),
    health1("./assets/h5.png"),
    health0("./assets/health10.png"),

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
   

    maxHealth = 8;
    health = 8;
    score = 0;
    hurtTimer = 0;
    nframes = 0;
    fps = 0;
    showfps = 1;
}

int score = 0;
bool gameOver = false;
static const int levelDurationFrames = 90 * 60;
static int levelTimerFrames = levelDurationFrames;
static bool levelPassed = false;

void checkCollisions();

void initGame()
{
    score = 0;
    gameOver = false;
    levelTimerFrames = levelDurationFrames;
    levelPassed = false;

    g.health = g.maxHealth;
    g.score = 0;
    g.hurtTimer = 0;

    initPlayer();      // harinaga.cpp
    initPowerups();    // falrowhani.cpp
    initGems();        // falrowhani.cpp
    propsGenerateInitial();
}

void updateLevelTimer()
{
    if (levelTimerFrames > 0) {
        levelTimerFrames--;
        if (levelTimerFrames <= 0) {
            levelTimerFrames = 0;
            levelPassed = true;
        }
    }
}



void renderGame()
{
   // g.gameBackground.show(g.xres/2, g.xres/2, g.yres/2, 0.0f);
    drawGems();
    drawPowerups();
    drawPlayer();
}

void checkCollisions()
{
    
}
//function to check if level is passed, called in cavein.cpp when pressing enter in game state
bool isLevelPassed()
{
    return levelPassed;
}
//function to get remaining time in seconds, called in cavein.cpp to display timer 
int getLevelTimeRemaining()
{
    return (levelTimerFrames + 59) / 60;
}
//function to render game HUD, called in cavein.cpp to display timer and level passed message
void renderGameDisplay()
{
    Rect r;
    r.left = 20;
    r.bot = g.yres - 30;
    r.center = 0;

    const int totalSeconds = getLevelTimeRemaining();
    const int minutes = totalSeconds / 60;
    const int seconds = totalSeconds % 60;
    char timerText[64];
    snprintf(timerText, sizeof(timerText), "Time: %d:%02d", minutes, seconds);
    ggprint(&r, 24, 16, 0x00ffffff, timerText);

    if (levelPassed) {
        Rect passed;
        passed.left = g.xres / 2;
        passed.bot = g.yres / 2 + 20;
        passed.center = 1;
        ggprint(&passed, 28, 18, 0x0000ff00, "Level Passed!");
        ggprint(&passed, 20, 18, 0x00ffffff, "Next Level");
        ggprint(&passed, 18, 18, 0x00ffffff, "Press ENTER to continue");
    }
}

void renderEndScreen()
{
    Rect r;

    g.background.show(g.xres/2, g.xres/2, g.yres/2, 0.0f);

    r.left = g.xres / 2;
    r.bot = g.yres / 2 + 40;
    r.center = 1;

    ggprint(&r, 30, 18, 0x00ff4444, "You Lost!");
    ggprint(&r, 20, 18, 0x00ffffff, "Score: %d", g.score);
    ggprint(&r, 18, 18, 0x00ffffff, "Press ENTER to continue");
}
