#ifndef _JGARIBAY_H_
#define _JGARIBAY_H_
#include <AL/alut.h>
#include "image.h"

enum SoundType {
	SOUND_CLICK = 0,
	SOUND_MENU,
	SOUND_SCROLL,
	SOUND_GEM,
	SOUND_HURT
};

enum PowerupType {
	POWER_SPEED,
	POWER_SHIELD,
	POWER_HEART
};

class Sound {
	private:
		ALuint source;
		ALuint buffer;
		float gain;
		float pitch;
		bool loop;
	public:
		Sound(const char *file, float gain, float pitch, bool loop);
		void play();
		void stop();
		~Sound();

};

class Openal {
	public:
		Openal();
		~Openal();
};

class Powerup {
	private:
		float x, y;
		float w, h;
		bool active;

		PowerupType type;
		Image *image;
		Sound *sound;
	public:
		void init(PowerupType type, Image *image, Sound *sound, float x,
				float y, float w, float h);
		void draw();
		bool collides(float px, float py, float pw, float ph);
		void activate();
		void update(float px, float py, float pw, float ph);
		bool isActive();
};

// global variable declarations
extern Sound  clickSound;
extern Sound   menuSound;
extern Sound   gameSound;
extern Sound scrollSound;
extern Sound    gemSound;
extern Sound   hurtSound;
extern Sound   overSound;

// function prototypes
void test();
void renderHelp();
void init_misc();
void drawStatusEffects();
void spawnPowerup(PowerupType type, Image *image, Sound *sound, float x,
		float y, float w, float h);
void initPowerups();
void drawPowerups();
void updatePowerups();
void drawHUD();

#endif // _JGARIBAY_H_

