#include "game.h"
#include "harinaga.h"
#include "jgaribay.h"
#include "falrowhani.h"
#include "fonts.h"
#include <cstring>
#include <cstdio>

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

    initPlayer();      // harinaga.cpp
    initPowerups();    // falrowhani.cpp
    initGems();        // falrowhani.cpp
}
// Called in cavein.cpp when pressing enter in menu state to start the game
void gamePhysics()
{
    if (levelPassed) {
        return;
    }

    updatePlayer();
    updatePowerups();
    updateGems();

    checkCollisions();
    // Update level timer
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
