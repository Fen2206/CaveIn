#ifndef GAME_H
#define GAME_H

#include "image.h"

enum GameState {
    STATE_TITLE,
    STATE_MENU,
    STATE_GAME,
    STATE_SETTINGS,
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


    float scale;
    int menuSelection;
    GameState state;


    Global();
};

extern Global g;

void initGame();
void gamePhysics();
void updateLevelTimer();
void renderGame();
bool isLevelPassed();
int getLevelTimeRemaining();
void renderGameDisplay();
void renderEndScreen();

#endif
