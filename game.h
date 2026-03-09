#ifndef GAME_H
#define GAME_H


#include "image.h"

void initGame();
void gamePhysics();
void renderGame();

enum GameState {
    STATE_TITLE,
    STATE_MENU,
    STATE_GAME,
    STATE_SETTINGS,
    STATE_EXIT
};

class Global {
public:
    int xres, yres;
    char keys[65536];
    int mouse_cursor_on;

    Image background;
    Image game;
    Image diamond;
    Image spike;

    float scale;
    int menuSelection;
    GameState state;

    Global();  
};


extern Global g;

#endif