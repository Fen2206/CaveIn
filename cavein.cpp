/*
 * program:     cavein.cpp
 * modified by: fenoon alrowhani, henry arinaga, joshua garibay
 * date:        spring 2026
 *
 * framework:       asteroids.cpp
 * author:          Gordon Griesel
 * date:            2014 - 2025
 * mod spring 2015: added constructors
 * description:     This program is a game starting point for a 3350 project.
 */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include <cmath>
#include <X11/Xlib.h>
#include <GL/gl.h> 		
#include <GL/glu.h>

// #include <X11/Xutil.h>
// #include <GL/gl.h>
// #include <GL/glu.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"
#include "image.h"
#include "harinaga.h"
#include "falrowhani.h"
#include "jgaribay.h"
#include "game.h"
#include "input.h"

// defined types
typedef float Flt;
typedef float Vec[3];
typedef Flt Matrix[4][4];

// macros
#define rnd() (((Flt)rand()) / (Flt)RAND_MAX)
#define random(a) (rand() % a)
#define VecZero(v) (v)[0] = 0.0, (v)[1] = 0.0, (v)[2] = 0.0
#define MakeVector(x, y, z, v) (v)[0] = (x), (v)[1] = (y), (v)[2] = (z)
#define VecCopy(a, b) \
    (b)[0] = (a)[0];  \
    (b)[1] = (a)[1];  \
    (b)[2] = (a)[2]
#define VecDot(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])
#define VecSub(a, b, c)       \
    (c)[0] = (a)[0] - (b)[0]; \
    (c)[1] = (a)[1] - (b)[1]; \
    (c)[2] = (a)[2] - (b)[2]
// constants
const float timeslice = 1.0f;
const float gravity = -0.2f;
#define PI 3.141592653589793
#define ALPHA 1
const int MAX_BULLETS = 11;
const Flt MINIMUM_ASTEROID_SIZE = 60.0;

//-----------------------------------------------------------------------------
// Setup timers
const double physicsRate = 1.0 / 60.0;
const double oobillion = 1.0 / 1e9;
extern struct timespec timeStart, timeCurrent;
extern struct timespec timePause;
extern double physicsCountdown;
extern double timeSpan;
extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);
//-----------------------------------------------------------------------------


Global g;

char g_keys[65536] = {0};

// X Windows variables
class X11_wrapper
{
private:
    Display *dpy;
    Window win;
    GLXContext glc;

public:
    X11_wrapper() {}
    X11_wrapper(int w, int h)
    {
        GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
        // GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
        XSetWindowAttributes swa;
        setup_screen_res(g.xres, g.yres);
        dpy = XOpenDisplay(NULL);
        if (dpy == NULL)
        {
            std::cout << "\n\tcannot connect to X server" << std::endl;
            exit(EXIT_FAILURE);
        }
        Window root = DefaultRootWindow(dpy);
        XWindowAttributes getWinAttr;
        XGetWindowAttributes(dpy, root, &getWinAttr);
        int fullscreen = 0;
        g.xres = w;
        g.yres = h;
        if (!w && !h)
        {
            // Go to fullscreen.
            g.xres = getWinAttr.width;
            g.yres = getWinAttr.height;
            // When window is fullscreen, there is no client window
            // so keystrokes are linked to the root window.
            XGrabKeyboard(dpy, root, False,
                          GrabModeAsync, GrabModeAsync, CurrentTime);
            fullscreen = 1;
        }
        XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
        if (vi == NULL)
        {
            std::cout << "\n\tno appropriate visual found\n"
                      << std::endl;
            exit(EXIT_FAILURE);
        }
        Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                         PointerMotionMask | MotionNotify | ButtonPress | ButtonRelease |
                         StructureNotifyMask | SubstructureNotifyMask;
        unsigned int winops = CWBorderPixel | CWColormap | CWEventMask;
        if (fullscreen)
        {
            winops |= CWOverrideRedirect;
            swa.override_redirect = True;
        }
        win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
                            vi->depth, InputOutput, vi->visual, winops, &swa);
        // win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
        // vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
        set_title();
        glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
        glXMakeCurrent(dpy, win, glc);
        show_mouse_cursor(0);
    }
    ~X11_wrapper()
    {
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
    }
    void set_title()
    {
        // Set the window title bar.
        XMapWindow(dpy, win);
        XStoreName(dpy, win, "Cave In!");
    }
    void check_resize(XEvent *e)
    {
        // The ConfigureNotify is sent by the
        // server if the window is resized.
        if (e->type != ConfigureNotify)
            return;
        XConfigureEvent xce = e->xconfigure;
        if (xce.width != g.xres || xce.height != g.yres)
        {
            // Window size did change.
            reshape_window(xce.width, xce.height);
        }
    }
    void draw_text(int x, int y, const char *str)
{
    int yx11 = g.yres - y;

    GC gc = XCreateGC(dpy, win, 0, NULL);
    XSetForeground(dpy, gc, 0x00ffffff);
    XDrawString(dpy, win, gc, x, yx11, str, (int)strlen(str));
    XFreeGC(dpy, gc);
}
    void reshape_window(int width, int height)
    {
        // window has been resized.
        setup_screen_res(width, height);
        glViewport(0, 0, (GLint)width, (GLint)height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glOrtho(0, g.xres, 0, g.yres, -1, 1);
        set_title();
    }
    void setup_screen_res(const int w, const int h)
    {
        g.xres = w;
        g.yres = h;
    }
    void swapBuffers()
    {
        glXSwapBuffers(dpy, win);
    }
    bool getXPending()
    {
        return XPending(dpy);
    }
    XEvent getXNextEvent()
    {
        XEvent e;
        XNextEvent(dpy, &e);
        return e;
    }
    void set_mouse_position(int x, int y)
    {
        XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
    }

    void show_mouse_cursor(const int onoff)
    {
        // printf("show_mouse_cursor(%i)\n", onoff); fflush(stdout);
        if (onoff)
        {
            // this removes our own blank cursor.
            XUndefineCursor(dpy, win);
            return;
        }
        // vars to make blank cursor
        Pixmap blank;
        XColor dummy;
        char data[1] = {0};
        Cursor cursor;
        // make a blank cursor
        blank = XCreateBitmapFromData(dpy, win, data, 1, 1);
        if (blank == None)
            std::cout << "error: out of memory." << std::endl;
        cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
        XFreePixmap(dpy, blank);
        // this makes the cursor. then set it using this function
        XDefineCursor(dpy, win, cursor);
        // after you do not need the cursor anymore use this function.
        // it will undo the last change done by XDefineCursor
        //(thus do only use ONCE XDefineCursor and then XUndefineCursor):
    }
} x11(g.xres, g.yres);
// ---> for fullscreen x11(0, 0);

// function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();
void renderTitle();
void renderMenu();
void renderGame();
void renderScrollingGameBackground();
void gamePhysics();
void renderHealth();
void renderSettings();

//==========================================================================
// M A I N
//==========================================================================
int main(int argc, char *argv[])
{
    // logOpen();
    srand(time(NULL));
    int targetfps = 60;
    double frameTime = 1.0 / targetfps;
    if (argc == 2) {
	    targetfps = atoi(argv[1]);
	    frameTime = 1.0 / targetfps;
	    printf("fps will be %i\n", targetfps);
    } else {
	    printf("Usage: %s <fps>\n", argv[0]);
	    printf("Example: %s 30 <--- fps will be set to 30\n", argv[0]);
	    exit(0);
    }

    init_opengl();
    init_misc();
    clock_gettime(CLOCK_REALTIME, &timePause);
    clock_gettime(CLOCK_REALTIME, &timeStart);
    x11.set_mouse_position(g.xres/2, g.yres/2);
    x11.show_mouse_cursor(g.mouse_cursor_on);
    menuSound.play();
    int done = 0;
    int seconds = time(NULL);
    while (!done)
    {
    	timespec frameStart, frameEnd;
	double frameSpan, sleepTime;
    	clock_gettime(CLOCK_REALTIME, &frameStart);
	while (x11.getXPending())
        {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            check_mouse(&e);
            done = check_keys(&e);
        }
        clock_gettime(CLOCK_REALTIME, &timeCurrent);
        timeSpan = timeDiff(&timeStart, &timeCurrent);
        timeCopy(&timeStart, &timeCurrent);
        physicsCountdown += timeSpan;
        while (physicsCountdown >= physicsRate)
        {
            physics();
            physicsCountdown -= physicsRate;
        }
        render();
	++g.nframes;
	int tmp = time(NULL);
	if (seconds != tmp) {
		g.fps = g.nframes;
		g.nframes = 0;
		seconds = tmp;
	}
        x11.swapBuffers();
	    	clock_gettime(CLOCK_REALTIME, &frameEnd);
	        frameSpan = timeDiff(&frameStart, &frameEnd);
		sleepTime = frameTime - frameSpan;
		if (sleepTime > 0.0) {
			const double MICROSECONDS = 1000000.0;
			usleep(sleepTime*MICROSECONDS); // convert from seconds to micros
		}
    }
    cleanup_fonts();
    // logClose();
    return 0;
}
float resolution_scale(Image *img)
{
    if (!img || img->width == 0 || img->height == 0)
        return 1.0f;
    float sx = (float)g.xres / (float)img->width;
    float sy = (float)g.yres / (float)img->height;
    return (sx > sy) ? sx : sy;
}

void init_opengl(void)
{
    // OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    // Initialize matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // This sets 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //
    // Clear the screen to black
    glClearColor(0.0, 0.0, 0.0, 1.0);
    // Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
    g.background.init_gl();
    //g.rock.init_gl();
    g.game.init_gl();
    g.diamond.init_gl();
    g.spike.init_gl();
    g.scale = resolution_scale(&g.background);
    titleAnimationInit(g.xres, g.yres);
    propsGenerateInitial();
    g.health8.init_gl();
    g.health7.init_gl();
    g.health6.init_gl();
    g.health5.init_gl();
    g.health4.init_gl();
    g.health3.init_gl();
    g.health2.init_gl();
    g.health1.init_gl();
    g.health0.init_gl();
    g.fireRock.init_gl(); 
    g.fireImpact.init_gl();
    //titleAnimationInit(g.xres, g.yres);
}

void normalize2d(Vec v)
{
    Flt len = v[0] * v[0] + v[1] * v[1];
    if (len == 0.0f)
    {
        v[0] = 1.0;
        v[1] = 0.0;
        return;
    }
    len = 1.0f / sqrt(len);
    v[0] *= len;
    v[1] *= len;
}
void check_mouse(XEvent *e)
{
    // Did the mouse move?
    // Was a mouse button clicked?
    static int savex = 0;
    static int savey = 0;
    //
    static int ct = 0;
    // std::cout << "m" << std::endl << std::flush;
    if (e->type == ButtonRelease)
        return;

    // keys[XK_Up] = 0;
    if (savex != e->xbutton.x || savey != e->xbutton.y)
    {
        // the mouse moved
        // int xdiff = savex - e->xbutton.x;
        // int ydiff = savey - e->xbutton.y;
        savex = e->xbutton.x;
        savey = e->xbutton.y;
        if (++ct < 10)
        {
            return;
        }
        // std::cout << "savex: " << savex << std::endl << std::flush;
        // std::cout << "e->xbutton.x: " << e->xbutton.x << std::endl <<
        // std::flush;
        //
        //  If mouse cursor is on, it does not control the ship.
        //  It's a regular mouse.
        if (g.mouse_cursor_on)
            return;
        // printf("mouse move "); fflush(stdout);

        x11.set_mouse_position(g.xres, g.yres);
        savex = 200;
        savey = 200;
    }
}

int check_keys(XEvent *e)
{
    static int shift = 0;
    if (e->type != KeyRelease && e->type != KeyPress)
    {
        // not a keyboard event
        return 0;
    }
    int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);
    // Log("key: %i\n", key);
    if (e->type == KeyRelease)
    {
        g_keys[key] = 0;
        if (key == XK_Shift_L || key == XK_Shift_R)
            shift = 0;
        return 0;
    }
    if (e->type == KeyPress)
    {
        // std::cout << "press" << std::endl;
        g_keys[key] = 1;
        if (key == XK_Shift_L || key == XK_Shift_R)
        {
            shift = 1;
            return 0;
        }
    }
    (void)shift;
    switch (key)
    {
    case XK_Return:
        clickSound.play();
        if (g.state == STATE_TITLE)
        {
            g.state = STATE_MENU;
        }
        else if (g.state == STATE_END) {
            if (g.endSelection == 0) {
                overSound.stop();
                g.level = 1;
                initGame();
                g.state = STATE_GAME;
            } else {
                overSound.stop();
                menuSound.play();
                g.level = 1;
                g.menuSelection = 0;
                g.state = STATE_MENU;
            }
        }
        else if (g.state == STATE_MENU)
        {

            if (g.menuSelection == 0) {
                g.state = STATE_GAME;
                g.level = 1;
                initGame();
            }

            else if (g.menuSelection == 1)
            {
                g.state = STATE_SETTINGS;
            }
            else if (g.menuSelection == 2) {
                g.state = STATE_HELP;
            }
            else if (g.menuSelection == 3)
            {
                return 1;
            }
        } else if (g.state == STATE_PAUSED) {
            if (g.pausedSelection == 0)
                g.state = STATE_GAME;
            else if (g.pausedSelection == 1)
                return 1;
	}
        //if in game and level passes press enter to restart
        else if (g.state == STATE_GAME && isLevelPassed())
        {
            g.level++;
            initGame();
        }
        else if (g.state == STATE_HELP || g.state == STATE_SETTINGS)
        {
            g.state = STATE_MENU;
        }

        break;
    case XK_w:
    case XK_Up:
        if (g.state == STATE_MENU)
        {
        	scrollSound.play();
            g.menuSelection--;
            if (g.menuSelection < 0)
                g.menuSelection = 3;
        }
        else if (g.state == STATE_END)
        {
            scrollSound.play();
            g.endSelection--;
            if (g.endSelection < 0)
                g.endSelection = 1;
        } else if (g.state == STATE_PAUSED) {
            scrollSound.play();
            g.pausedSelection--;
            if (g.pausedSelection < 0)
                g.pausedSelection = 1;
	}
        break;
    case XK_s:
    case XK_Down:
        if (g.state == STATE_MENU) {
        	scrollSound.play();
            g.menuSelection++;
            if (g.menuSelection > 3)
                g.menuSelection = 0;
        } else if (g.state == STATE_END) {
        	scrollSound.play();
            g.endSelection++;
            if (g.endSelection > 1)
                g.endSelection = 0;
        } else if (g.state == STATE_PAUSED) {
        	scrollSound.play();
		g.pausedSelection++;
		if (g.pausedSelection > 1)
			g.pausedSelection = 0;
	}
        break;
    case XK_Escape:
	if (g.state == STATE_GAME)
		g.state = STATE_PAUSED;
	else if (g.state == STATE_PAUSED)
		g.state = STATE_GAME;
	 break;
    case XK_m:
        g.mouse_cursor_on = !g.mouse_cursor_on;
        x11.show_mouse_cursor(g.mouse_cursor_on);
        break;
    case XK_h:
        break;
    case XK_equal:
        break;
    case XK_minus:
        break;
	case XK_f:
		g.showfps = !g.showfps;
		break;
	case XK_i:
		g.debugMode = !g.debugMode;
		printf("debug mode: %i", g.debugMode);
		break;
    }
    return 0;
}




void physics()
{
if (g.state == STATE_TITLE)
        titleAnimationUpdate(gravity);

if (g.state == STATE_GAME)
    gamePhysics();
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    switch (g.state)
    {
	    case STATE_TITLE:
		    renderTitle();
		    break;
	    case STATE_MENU:
		    renderMenu();
		    break;
	    case STATE_END:
		    renderEndScreen();
		    break;
	    case STATE_GAME:
		    renderScrollingGameBackground();
		    // g.game.show(g.xres/2, g.xres/2, g.yres/2, 0.0f); 
		    renderGame();
		    propsRender();
		    renderHealth();
		    renderGameDisplay();
		    break; 
	    case STATE_HELP:
		    renderHelp();
		    break;
	    case STATE_SETTINGS:
		    renderSettings();
		    break;
	    case STATE_PAUSED:
		    renderPaused();
		    break;
	    case STATE_EXIT:
		    exit(0);
		    break;
    }
}


void renderHealth()
{
    extern bool gameOver;
    Image *bar = &g.health0;

    switch (g.health) {
        case 8: bar = &g.health8; break;
        case 7: bar = &g.health7; break;
        case 6: bar = &g.health6; break;
        case 5: bar = &g.health5; break;
        case 4: bar = &g.health4; break;
        case 3: bar = &g.health3; break;
        case 2: bar = &g.health2; break;
        case 1: bar = &g.health1; break;
        case 0:
            bar = &g.health0;
            gameOver = true;
            g.endSelection = 0;
            gameSound.stop();
            overSound.play();
            g.state = STATE_END;
            break;
    }

    bar->show(120.0f, 110, g.yres - 35, 0.0f, 0);

    g.diamond.show(18.0f, 30, g.yres - 70, 0.0f, 0);

    //char str[64];
    //sprintf(str, "Score: %d", g.score);
}


void renderScrollingGameBackground()
{
    float offsetY = std::fmod(g.cameraY, (float)g.yres);
    if (offsetY < 0.0f) {
        offsetY += g.yres;
    }

    float centerX = g.xres * 0.5f;
    float centerY = g.yres * 0.5f;

    for (int i = -1; i <= 1; i++) {
        float drawY = centerY - offsetY + i * g.yres;
        g.game.show(g.xres / 2, (int)centerX, (int)drawY, 0.0f);
    }
}
void renderTitle()
{
   

    g.background.show(g.xres/2, g.xres/2, g.yres/2, 0.0f);
    titleAnimationRender();

    Rect r;
    r.bot = g.yres/2 - 10;
    r.left = g.xres/2;
    r.center = 1;

    Rect r2;
    r2.bot = 250;
    r2.left = g.xres/2;
    r2.center = 1;

    ggprint(&r2, 24, 16, 0x00ffffff, "Press ENTER to continue");
}

void renderMenu()
{
    Rect r;
    r.bot = g.yres - 200;
    r.left = g.xres / 2;
    r.center = 1;

    ggprint(&r, 32, 32, 0x00ffffff, "MAIN MENU");

    const int NOPTIONS = 4;
    const char *options[NOPTIONS] = {
        "Start Game",
        "Settings",
	"How to play",
        "Exit"
    };

    for (int i = 0; i < NOPTIONS; i++)
    {
        if (i == g.menuSelection)
            ggprint(&r, 24, 24, 0x0000ff00, options[i]);
        else
            ggprint(&r, 24, 24, 0x00ffffff, options[i]);
    }
}

void renderSettings()
{
    Rect r;
    r.bot = g.yres - 200;
    r.left = g.xres / 2;
    r.center = 1;

    ggprint(&r, 32, 32, 0x00ffffff, "SETTINGS");
    ggprint(&r, 24, 24, 0x0000ff00, "Back");
    
    const int NOPTIONS = 1;
    const char *options[NOPTIONS] = {
        "show fps - f",
    };

    for (int i = 0; i < NOPTIONS; i++)
    {
        if (i == g.menuSelection)
            ggprint(&r, 24, 24, 0x0000ff00, options[i]);
        else
            ggprint(&r, 24, 24, 0x00ffffff, options[i]);
    }
}
