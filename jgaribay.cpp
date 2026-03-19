#include <cstdio>
#include <AL/alut.h>
#include <unistd.h>
#include "jgaribay.h"
#include "input.h" 	// for keys array
#include "game.h" 	// for global class

Openal oal;
Sound click ("./sounds/click.wav",  1.0, 1.0, 0);
Sound menu  ("./sounds/menu.wav",   0.5, 1.0, 1);
Sound scroll("./sounds/scroll.wav", 1.0, 1.0, 0);
Sound gem   ("./sounds/gem.wav",    1.0, 1.0, 0);
Sound hurt  ("./sounds/hurt.wav",   1.0, 1.0, 0);

void test()
{
	if (g_keys[XK_y])
		hurt.play();
}

Sound::Sound(const char *file, float gain, float pitch, bool loop)
{
	this->gain = gain;
	this->pitch = pitch;
	this->loop = loop;

	buffer = alutCreateBufferFromFile(file);
	alGenSources(1, &source);

	alSourcei(source, AL_BUFFER, buffer);
	alSourcef(source, AL_GAIN, gain);
	alSourcef(source, AL_PITCH, pitch);
	alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);

	if (alGetError() != AL_NO_ERROR)
		printf("Error\n");
}

void Sound::play()
{
	alSourcePlay(source);
}

Sound::~Sound()
{
	alDeleteSources(1, &source); // first delete the source
	alDeleteBuffers(1, &buffer); // delete the buffer
}

Openal::Openal()
{
	//Get started right here.
	alutInit(0, NULL);
	if (alGetError() != AL_NO_ERROR) {
		printf("ERROR: starting sound.\n");
	}
	//Clear error state
	alGetError();
	//Setup the listener.
	//Forward and up vectors are used.
	float vec[6] = {0.0f,0.0f,1.0f, 0.0f,1.0f,0.0f};
	alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
	alListenerfv(AL_ORIENTATION, vec);
	alListenerf(AL_GAIN, 1.0f);
}

Openal::~Openal()
{
	//Close out OpenAL itself.
	//unsigned int alSampleSet;
	ALCcontext *Context;
	ALCdevice *Device;
	//Get active context
	Context=alcGetCurrentContext();
	//Get device for active context
	Device=alcGetContextsDevice(Context);
	//Disable context
	alcMakeContextCurrent(NULL);
	//Release context(s)
	alcDestroyContext(Context);
	//Close device
	alcCloseDevice(Device);
}

