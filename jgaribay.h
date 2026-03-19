#ifndef JGARIBAY_H
#define JGARIBAY_H
#include <AL/alut.h>
#include "image.h"

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
		~Sound();

};

class Openal {
	public:
		Openal();
		~Openal();
};

class Powerup {
	private:
		Image img;
		Sound *sound;
		float x, y;
		float w, h;
		bool active;
	public:
		Powerup(const char *, Sound *sound, float x, float y, float w, float h);
		void draw();
		bool collides(float px, float py, float pw, float ph);
		void activate();
		void update();
		bool isActive();
};

// global variable declarations
extern Sound  clickSound;
extern Sound   menuSound;
extern Sound scrollSound;
extern Sound    gemSound;
extern Sound   hurtSound;

// function prototypes
void test();
void renderHelp();
void init_misc();

#endif

