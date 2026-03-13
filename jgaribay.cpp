#define USE_SOUND
#ifdef USE_SOUND
#include <cstdio>
#include <AL/alut.h>
#include <unistd.h>
#include "jgaribay.h"

const char *sound_files[] = {
    "./sounds/click.wav",
    "./sounds/menu_music.wav",
    "./sounds/switch.wav",
    "./sounds/gem.wav",
    "./sounds/hurt.wav"
};

int sound_loop[] = {
    0,
    1,
    0,
    0,
    0
};

float sound_gain[] = {
    1.0,
    0.5,
    1.0,
    1.0,
    1.0
};

class Openal {
    ALuint alSource[NSOUNDS];
    ALuint alBuffer[NSOUNDS];
    public:
    Openal() {
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
        //
        //Buffer holds the sound information.
        //alBuffer[0] = alutCreateBufferFromFile("./sounds/billiard.wav");
        
        for (int i = 0; i < NSOUNDS; i++) {
            alBuffer[i] = alutCreateBufferFromFile(sound_files[i]);

            alGenSources(1, &alSource[i]);
            alSourcei(alSource[i], AL_BUFFER, alBuffer[i]);

            alSourcef(alSource[i], AL_GAIN, sound_gain[i]);
            alSourcef(alSource[i], AL_PITCH, 1.0f);

            if (sound_loop[i])
                alSourcei(alSource[i], AL_LOOPING, AL_TRUE);
            else
                alSourcei(alSource[i], AL_LOOPING, AL_FALSE);
            if (alGetError() != AL_NO_ERROR) {
                printf("ERROR\n");
            }
        }

    }
    void playSound(int i)
    {
        alSourcePlay(alSource[i]);
    }
    ~Openal() {
        //First delete the source.
        alDeleteSources(1, &alSource[0]);
        alDeleteSources(1, &alSource[1]);
        //Delete the buffer
        alDeleteBuffers(1, &alBuffer[0]);
        alDeleteBuffers(1, &alBuffer[1]);
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
} oal;
#endif

void playSound(int s) {
#ifdef USE_SOUND
    oal.playSound(s);
#else
    if (s) {}
#endif
}

void initObstacles() {}
void updateObstacles() {}
void drawObstacles() {}

