#include <cstdio>
#include <AL/alut.h>
#include <unistd.h>
#include "jgaribay.h"
#include "input.h" 	// for keys array
#include "game.h" 	// for global class
#include "fonts.h" 	// for ggprint
#include <cstring>

Openal oal;
Sound  clickSound("./sounds/click.wav",  1.0, 1.0, 0);
Sound   menuSound("./sounds/menu.wav",   0.5, 1.0, 1);
Sound scrollSound("./sounds/scroll.wav", 1.0, 1.0, 0);
Sound    gemSound("./sounds/gem.wav",    1.0, 1.0, 0);
Sound   hurtSound("./sounds/hurt.wav",   1.0, 1.0, 0);
Powerup shieldPowerup(POWER_SHIELD, "./assets/shield.png", &gemSound, 100, 100, 32, 32);
Image shieldImage("./assets/shield.png"); // use this in the shield powerup

// ----- Sound class -----
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

// ----- Openal class -----
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
	ALCcontext *Context;
	ALCdevice *Device;
	Context = alcGetCurrentContext(); 		// get active context
	Device = alcGetContextsDevice(Context); 	// get device for active context
	alcMakeContextCurrent(NULL); 			// disable context
	alcDestroyContext(Context); 			// release context(s)
	alcCloseDevice(Device); 			// close device
}

// ----- Powerup class -----
// note: two options, either make inherited classes for each type of powerup
// or make an enum instead and use that
Powerup::Powerup(PowerupType type, const char *img, Sound *s, float x, float y, float w, float h) : img(img)
{
	this->type = type;
	sound = s;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;

	this->img.init_gl();
	active = 1;
}

void Powerup::draw()
{
	// custom draw() implemented for my collision
	glBindTexture(GL_TEXTURE_2D, img.texture);
	glColor4f(0.0, 0.0, 0.0, 0.0);
	glPushMatrix();
		glTranslatef(x, y, 0.0f);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0f);
		glColor4ub(255, 255, 255, 255);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 1); glVertex2i(0, 0);
			glTexCoord2i(0, 0); glVertex2i(0, h);
			glTexCoord2i(1, 0); glVertex2i(w, h);
			glTexCoord2i(1, 1); glVertex2i(w, 0);
		glEnd();
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0); // unbinds texture
}

bool Powerup::collides(float px, float py, float pw, float ph)
{
	return px < x + w && px + pw > x && py < y + h && py + ph > y;
}

void Powerup::activate()
{
	sound->play();
	active = 0;
}

void Powerup::update()
{
	// do everything in here maybe
}

bool Powerup::isActive()
{
	return active;
}

// ----- Other functions -----
extern float px, py;
void test()
{
	float pw = 32.0f;
	float ph = 32.0f;
	float playerLeft = px - (pw / 2);
	float playerBottom = py - (ph / 2);

	if (g_keys[XK_y])
		hurtSound.play();
	shieldPowerup.draw();
	if (shieldPowerup.isActive() && shieldPowerup.collides(playerLeft, playerBottom, pw, ph)) {
		printf("collision!\n");
		shieldPowerup.activate();
	}
}

void init_misc()
{
	shieldImage.init_gl();
}

void renderHelp()
{
	Rect r;
	r.bot = g.yres - 40.0f;
	r.left = 50.0f;
	r.center = 0;

	const float imgWidth = 16.0 / 2;
	shieldImage.show(imgWidth, imgWidth + 10.0f, g.yres - 10.0f - imgWidth, 0.0f);
	const char *list[] = {
		"Objective: collect all gems before the timer ends use powerups ",
		"to help along the way",
		"Shield: resistance to the next obstacle",
                "Speed: speed for - seconds",
                "+1 Health: grants +1 health to player",
		""
	};

	int i = 0;
	// better way to loop through menu from another lab
	while (strlen(list[i]) > 0)
		ggprint(&r, 10, 40, 0x00ffaaff, list[i++]);
}

