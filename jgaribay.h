#ifndef JGARIBAY_H
#define JGARIBAY_H
#include <AL/alut.h>

#define NSOUNDS 5

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

// global variable declarations
extern Sound click, menu, scroll, gem, hurt;

// function prototypes
void test();

#endif

