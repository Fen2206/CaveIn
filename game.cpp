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
    endSelection = 0;
    pausedSelection = 0;


    cameraX = 0.0f;
    cameraY = 0.0f;
    level = 1;
    propsGenerateInitial();
   

    maxHealth = 8;
    health = 8;
    score = 0;
    hurtTimer = 0;
    nframes = 0;
    fps = 0;
    showfps = 1;
    shieldTimer = 0;
    speedTimer = 0;
    debugMode = 0;
}

int score = 0;
bool gameOver = false;
static const int levelDurationFrames = 30 * 60;
static int levelTimerFrames = levelDurationFrames;
static bool levelPassed = false;
float speed = 4.0f;

void checkCollisions();

void initGame()
{
	menuSound.stop();
    gameSound.play();
    gameOver = false;
    levelTimerFrames = levelDurationFrames;
    levelPassed = false;

    if (g.level < 1)
        g.level = 1;
    if (g.level == 1) {
        score = 0;
        g.score = 0;
    }

    g.health = g.maxHealth;
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
    drawStatusEffects();
    drawPlayer();
	test();
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
    r.left = g.xres - 120.0f;
    r.bot = g.yres - 30.0f;
    r.center = 0;

    const int totalSeconds = getLevelTimeRemaining();
    const int minutes = totalSeconds / 60;
    const int seconds = totalSeconds % 60;
    char timerText[64];
    snprintf(timerText, sizeof(timerText), "Time: %d:%02d", minutes, seconds);
    ggprint(&r, 16, 20, 0x00ffffff, timerText);
    ggprint(&r, 16, 20, 0x00ffffff, "Level: %d", g.level);
    ggprint(&r, 16, 20, 0x00ffffff, "Score: %i", g.score);
    if (g.showfps)
    	ggprint(&r, 16, 20, 0x00ffffff, "[f] fps: %i", g.fps);

    if (levelPassed) {
        Rect passed;
        passed.left = g.xres / 2;
        passed.bot = g.yres / 2 + 20;
        passed.center = 1;
        ggprint(&passed, 28, 18, 0x0000ff00, "Level Passed!");
        ggprint(&passed, 20, 18, 0x00ffffff, "Next Level");
        ggprint(&passed, 18, 18, 0x00ffffff, "Press ENTER to continue");
    }
    drawHUD();
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

    const int NOPTIONS = 2;
    const char *options[NOPTIONS] = {
        "Retry",
        "Main Menu"
    };

    for (int i = 0; i < NOPTIONS; i++) {
        if (i == g.endSelection)
            ggprint(&r, 20, 18, 0x0000ff00, options[i]);
        else
            ggprint(&r, 20, 18, 0x00ffffff, options[i]);
    }
}
