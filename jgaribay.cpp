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
Sound   menuSound("./sounds/menu.wav",   0.6, 1.0, 1);
Sound   gameSound("./sounds/game.wav",   0.8, 1.0, 1);
Sound   overSound("./sounds/over.wav",   0.8, 1.0, 1);
Sound scrollSound("./sounds/scroll.wav", 1.0, 1.0, 0);
Sound    gemSound("./sounds/gem.wav",    1.0, 1.0, 0);
Sound   hurtSound("./sounds/hurt.wav",   1.0, 1.0, 0);
Sound shieldSound("./sounds/shield.wav", 1.0, 1.0, 0);
Sound  speedSound("./sounds/speed.wav",  1.0, 1.0, 0);
Sound  heartSound("./sounds/heart.wav",  1.0, 1.0, 0);
Image shieldImage("./assets/shield.png");
Image bubbleImage("./assets/bubble.png");
Image speedImage("./assets/speed.png");
Image heartImage("./assets/heart.png");
Powerup shieldPowerup(POWER_SHIELD, &shieldImage, &shieldSound, 200, 200, 32, 32);
Powerup  speedPowerup(POWER_SPEED, &speedImage, &speedSound, 250, 200, 32, 32);
Powerup  heartPowerup(POWER_HEART, &heartImage, &heartSound, 300, 200, 32, 32);

// ----- Sound class -----
// note: consider adding currentMusic variable
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

void Sound::stop()
{
	alSourceStop(source);
}

Sound::~Sound()
{
	alDeleteSources(1, &source); // first delete the source
	alDeleteBuffers(1, &buffer); // delete the buffer
}

// ----- Openal class -----
Openal::Openal()
{
	alutInit(0, NULL); 				// get started right here
	if (alGetError() != AL_NO_ERROR) {
		printf("ERROR: starting sound.\n");
	}
	alGetError(); 					// clear error state
	//Forward and up vectors are used.
	float vec[6] = {0.0f,0.0f,1.0f, 0.0f,1.0f,0.0f};
	alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f); 	// setup the listener
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
Powerup::Powerup(PowerupType type, Image *image, Sound *s, float x, float y, float w, float h)
{
	this->type = type;
	sound = s;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;

	this->image = image;
	active = 1;
}

void Powerup::draw()
{
	float sx = x - g.cameraX;
	float sy = y - g.cameraY;

	// custom draw() implemented for my collision
	glBindTexture(GL_TEXTURE_2D, image->texture);
	glColor4f(0.0, 0.0, 0.0, 0.0);
	glPushMatrix();
		glTranslatef(sx, sy, 0.0f);
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
	printf("collision!\n");
	sound->play();

	if (type == POWER_SHIELD) {
		g.shieldTimer = 120;
	} else if (type == POWER_HEART) {
		g.health += 1;
	}

	active = 0;
}

void Powerup::update(float px, float py, float pw, float ph)
{
	float playerLeft = px - (pw / 2);
	float playerBottom = py - (ph / 2);

	if (!active)
		return;

	if (collides(playerLeft, playerBottom, pw, ph)) {
		activate();
	}
}

bool Powerup::isActive()
{
	return active;
}

// ----- Other functions -----
extern float px, py;
void test()
{
	const float pw = 32.0f;
	const float ph = 32.0f;

	if (g_keys[XK_y])
		hurtSound.play();
	shieldPowerup.update(px, py, pw, ph);
	if (shieldPowerup.isActive())
		shieldPowerup.draw();

	speedPowerup.update(px, py, pw, ph);
	if (speedPowerup.isActive())
		speedPowerup.draw();

	heartPowerup.update(px, py, pw, ph);
	if (heartPowerup.isActive())
		heartPowerup.draw();
}

void init_misc()
{
	shieldImage.init_gl();
	bubbleImage.init_gl();
	speedImage.init_gl();
	heartImage.init_gl();
}

void renderHelp()
{
	Rect r;
	r.bot = g.yres - 40.0f;
	r.left = 50.0f;
	r.center = 0;

	const float imgWidth = 16.0;
	shieldImage.show(imgWidth, imgWidth + 10.0f, g.yres - 10.0f - imgWidth, 0.0f);
	const char *list[] = {
		"Objective: collect all gems before the timer ends use powerups ",
		"to help along the way",
		"Shield: grants resistanace to the player from obstacles 2s",
                "Speed: speed for - seconds",
                "Heart: grants health to the player",
		""
	};

	int i = 0;
	// better way to loop through menu from another lab
	while (strlen(list[i]) > 0)
		ggprint(&r, 10, 40, 0x00ffaaff, list[i++]);
}

void drawStatusEffects()
{
	float sx = px - g.cameraX;
	float sy = py - g.cameraY;
	const float imgWidth = 24.0;
	if (g.shieldTimer > 0)
		bubbleImage.show(imgWidth, sx, sy, 0.0f);
}

