#ifndef JGARIBAY_H
#define JGARIBAY_H

void initObstacles();
void updateObstacles();
void drawObstacles();

void playSound(int);

extern const char *sound_files[];
extern int sound_loop[];
extern float sound_gain[];

enum Sound {
    UI_CLICK = 0,
    MENU_MUSIC,
    UI_SWITCH,
    GEM_SPARKLE,
    PLAYER_HURT
};

const int NSOUNDS = 5;

#endif

