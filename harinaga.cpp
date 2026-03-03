#include "harinaga.h"
#include <GL/gl.h>
#include <cmath>
#include "defs.h"
#include <X11/keysym.h>

static float px = 100.0f;
static float py = 100.0f;
static float speed = 4.0f;

void initPlayer()
{
    px = 100.0f;
    py = 100.0f;
    speed = 4.0f;
}

void updatePlayer()
{   
    // keys for player movement
    if (g_keys[XK_Left] || g_keys[XK_a])
    {
        px -= speed;
    }
    if (g_keys[XK_Right] || g_keys[XK_d])
    {
        px += speed;
    }
    if (g_keys[XK_Up] || g_keys[XK_w])
    {
        py += speed;
    }
    if (g_keys[XK_Down] || g_keys[XK_s])
    {
        py -= speed;
    }

    // clamp player movement to the window boundaries
    if (px < 0.0f)
    {
        px = 0.0f;
    }
    if (px > 500.0f)
    {
        px = 500.0f;
    }
    if (py < 0.0f)
    {
        py = 0.0f;
    }
    if (py > 650.0f)
    {
        py = 650.0f;
    }
}

void drawPlayer()
{
    float w = 20.0f;
    float h = 20.0f;

    glPushMatrix();
    glTranslatef(px, py, 0.0f);

    glBegin(GL_QUADS);
    glVertex2f(-w, -h);
    glVertex2f(-w, h);
    glVertex2f(w, h);
    glVertex2f(w, -h);
    glEnd();

    glPopMatrix();
}