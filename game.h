/*
 * program:     cavein.cpp
 * modified by: fenoon alrowhani, henry arinaga, joshua garibay
 * date:        spring 2026
 */
#ifndef GAME_H
#define GAME_H

#include "image.h"

enum GameState {
    STATE_TITLE,
    STATE_MENU,
    STATE_GAME,
    STATE_HELP,
    STATE_SETTINGS,
    STATE_PAUSED,
    STATE_EXIT,
    STATE_END
};

class Global {
public:
    int xres, yres;
    char keys[65536];
    int mouse_cursor_on;
    float cameraX;
    float cameraY;
    int fps;
    int frameCount;
    time_t final; 
    int nframes;
    bool showfps;


    Image background;
    Image game;
    Image diamond;
    Image spike;
    Image health8;
    Image health7; 
    Image health6;

    Image health5;
    Image health4;
    Image health3;
    Image health2;
    Image health1;
    Image health0;
  

    Image fireRock; 
    Image fireImpact; 

    int health;
    int maxHealth;
    int score;
    int hurtTimer;
    int show_warning;
    int warning_timer;
    int shieldTimer;
    int speedTimer;
    int level;
    float stamina;
    float maxStamina;
    float staminaRegenRate;
    float dashStaminaCost;
    float staminaRegenDelay;
    float staminaRegenTimer;
    float scale;
    int menuSelection;
    int endSelection;
    int pausedSelection;
    GameState state;
    int debugMode;


    Global();
};

extern Global g;
extern float speed;

void initGame();
void gamePhysics();
void updateLevelTimer();
void renderGame();
bool isLevelPassed();
int getLevelTimeRemaining();
void renderGameDisplay();
void renderEndScreen();

#endif
