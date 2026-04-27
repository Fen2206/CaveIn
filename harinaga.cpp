/*
 * program:     cavein.cpp
 * modified by: henry arinaga
 * date:        spring 2026
 */
#include "harinaga.h"
#include <GL/gl.h>
#include <cmath>
#include "input.h"
#include <X11/keysym.h>
#include "image.h"
#include "game.h"
#include "jgaribay.h"
// Direction enum for player facing direction,
// used for selecting the correct sprite frames and dash direction
enum Direction
{
    DIR_S,
    DIR_SW,
    DIR_W,
    DIR_NW,
    DIR_N,
    DIR_NE,
    DIR_E,
    DIR_SE
};

// Player state variables
static Direction dir = DIR_S;
float playerPosX = 100.0f;
float playerPosY = 100.0f;
static float sparkleTimer = 0.0f;
static float sparkleAngle = 0.0f;
static const float sparkleDuration = 0.6f;
static const float baseSpeed = 4.0f;
static const float dashPeakSpeed = 12.0f;
static const float dashDuration = 0.28f;
static const float dashSigma = 0.075f;
static const float dashCooldownDuration = 0.45f;
static bool hurtActive = false;
static int hurtFrame = 0;
static int hurtFrameCounter = 0;
static const int hurtFrameCount = 6;
static const int hurtFrameDelay = 2;
static int hurtCooldown = 0;
static const int hurtCooldownDuration = 60;
static bool dashActive = false;
static bool dashKeyHeld = false;
static float dashTimer = 0.0f;
static float dashCooldownTimer = 0.0f;
static float dashDirX = 0.0f;
static float dashDirY = -1.0f;

// Sprite frames for each direction and hurt state
static Image southFrames[4] = {
    Image("./assets/character_sprites/male_character/south/south_00.png"),
    Image("./assets/character_sprites/male_character/south/south_01.png"),
    Image("./assets/character_sprites/male_character/south/south_02.png"),
    Image("./assets/character_sprites/male_character/south/south_03.png")};

static Image northFrames[4] = {
    Image("./assets/character_sprites/male_character/north/north_00.png"),
    Image("./assets/character_sprites/male_character/north/north_01.png"),
    Image("./assets/character_sprites/male_character/north/north_02.png"),
    Image("./assets/character_sprites/male_character/north/north_03.png")};

static Image eastFrames[4] = {
    Image("./assets/character_sprites/male_character/east/east_00.png"),
    Image("./assets/character_sprites/male_character/east/east_01.png"),
    Image("./assets/character_sprites/male_character/east/east_02.png"),
    Image("./assets/character_sprites/male_character/east/east_03.png")};

static Image westFrames[4] = {
    Image("./assets/character_sprites/male_character/west/west_00.png"),
    Image("./assets/character_sprites/male_character/west/west_01.png"),
    Image("./assets/character_sprites/male_character/west/west_02.png"),
    Image("./assets/character_sprites/male_character/west/west_03.png")};

static Image neFrames[4] = {
    Image("./assets/character_sprites/male_character/northeast/northeast_00.png"),
    Image("./assets/character_sprites/male_character/northeast/northeast_01.png"),
    Image("./assets/character_sprites/male_character/northeast/northeast_02.png"),
    Image("./assets/character_sprites/male_character/northeast/northeast_03.png")};

static Image nwFrames[4] = {
    Image("./assets/character_sprites/male_character/northwest/northwest_00.png"),
    Image("./assets/character_sprites/male_character/northwest/northwest_01.png"),
    Image("./assets/character_sprites/male_character/northwest/northwest_02.png"),
    Image("./assets/character_sprites/male_character/northwest/northwest_03.png")};

static Image seFrames[4] = {
    Image("./assets/character_sprites/male_character/southeast/southeast_00.png"),
    Image("./assets/character_sprites/male_character/southeast/southeast_01.png"),
    Image("./assets/character_sprites/male_character/southeast/southeast_02.png"),
    Image("./assets/character_sprites/male_character/southeast/southeast_03.png")};

static Image swFrames[4] = {
    Image("./assets/character_sprites/male_character/southwest/southwest_00.png"),
    Image("./assets/character_sprites/male_character/southwest/southwest_01.png"),
    Image("./assets/character_sprites/male_character/southwest/southwest_02.png"),
    Image("./assets/character_sprites/male_character/southwest/southwest_03.png")};

static Image hurtSouthFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/south/south_00.png"),
    Image("./assets/character_sprites/male_character_hurt/south/south_01.png"),
    Image("./assets/character_sprites/male_character_hurt/south/south_02.png"),
    Image("./assets/character_sprites/male_character_hurt/south/south_03.png"),
    Image("./assets/character_sprites/male_character_hurt/south/south_04.png"),
    Image("./assets/character_sprites/male_character_hurt/south/south_05.png")};

static Image hurtNorthFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/north/north_00.png"),
    Image("./assets/character_sprites/male_character_hurt/north/north_01.png"),
    Image("./assets/character_sprites/male_character_hurt/north/north_02.png"),
    Image("./assets/character_sprites/male_character_hurt/north/north_03.png"),
    Image("./assets/character_sprites/male_character_hurt/north/north_04.png"),
    Image("./assets/character_sprites/male_character_hurt/north/north_05.png")};

static Image hurtEastFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/east/east_00.png"),
    Image("./assets/character_sprites/male_character_hurt/east/east_01.png"),
    Image("./assets/character_sprites/male_character_hurt/east/east_02.png"),
    Image("./assets/character_sprites/male_character_hurt/east/east_03.png"),
    Image("./assets/character_sprites/male_character_hurt/east/east_04.png"),
    Image("./assets/character_sprites/male_character_hurt/east/east_05.png")};

static Image hurtWestFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/west/west_00.png"),
    Image("./assets/character_sprites/male_character_hurt/west/west_01.png"),
    Image("./assets/character_sprites/male_character_hurt/west/west_02.png"),
    Image("./assets/character_sprites/male_character_hurt/west/west_03.png"),
    Image("./assets/character_sprites/male_character_hurt/west/west_04.png"),
    Image("./assets/character_sprites/male_character_hurt/west/west_05.png")};

static Image hurtNeFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_00.png"),
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_01.png"),
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_02.png"),
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_03.png"),
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_04.png"),
    Image("./assets/character_sprites/male_character_hurt/northeast/northeast_05.png")};

static Image hurtNwFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_00.png"),
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_01.png"),
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_02.png"),
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_03.png"),
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_04.png"),
    Image("./assets/character_sprites/male_character_hurt/northwest/northwest_05.png")};

static Image hurtSeFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_00.png"),
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_01.png"),
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_02.png"),
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_03.png"),
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_04.png"),
    Image("./assets/character_sprites/male_character_hurt/southeast/southeast_05.png")};

static Image hurtSwFrames[6] = {
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_00.png"),
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_01.png"),
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_02.png"),
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_03.png"),
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_04.png"),
    Image("./assets/character_sprites/male_character_hurt/southwest/southwest_05.png")};

// Global frame counters for animation
static int currentFrame = 0;
static int frameCounter = 0;

// Function to set player direction based on movement input,
// used for selecting sprite frames and dash direction
static void setDirectionFromMovement(float dx, float dy)
{
    // Determine direction based on the signs of dx and dy
    if (dx > 0 && dy > 0)
        dir = DIR_NE;
    else if (dx < 0 && dy > 0)
        dir = DIR_NW;
    else if (dx > 0 && dy < 0)
        dir = DIR_SE;
    else if (dx < 0 && dy < 0)
        dir = DIR_SW;
    else if (dx > 0)
        dir = DIR_E;
    else if (dx < 0)
        dir = DIR_W;
    else if (dy > 0)
        dir = DIR_N;
    else if (dy < 0)
        dir = DIR_S;
}

// Function to get the normalized direction vector for dashing based on the current direction enum
// This is used when the player initiates a dash without any movement input, so they dash in the direction they're currently facing
static void getDirectionVector(Direction direction, float &dx, float &dy)
{
    switch (direction)
    {
    case DIR_S:
        dx = 0.0f;
        dy = -1.0f;
        break;
    case DIR_SW:
        dx = -0.7071f;
        dy = -0.7071f;
        break;
    case DIR_W:
        dx = -1.0f;
        dy = 0.0f;
        break;
    case DIR_NW:
        dx = -0.7071f;
        dy = 0.7071f;
        break;
    case DIR_N:
        dx = 0.0f;
        dy = 1.0f;
        break;
    case DIR_NE:
        dx = 0.7071f;
        dy = 0.7071f;
        break;
    case DIR_E:
        dx = 1.0f;
        dy = 0.0f;
        break;
    case DIR_SE:
        dx = 0.7071f;
        dy = -0.7071f;
        break;
    }
}
// https://www.datacamp.com/tutorial/gaussian-distribution
/*  Gaussian function for dash speed curve
    This creates a smooth acceleration and deceleration effect for the dash.
    The speed peaks at dashPeakSpeed around the middle of the dash duration. */
static float getGaussianDashSpeed(float elapsed) // elapsed is the time since the dash started
{
    const float mu = dashDuration * 0.5f;                                       // The midpoint of the dash where speed should peak
    const float offset = elapsed - mu;                                          // Center the curve around the midpoint of the dash
    const float exponent = -(offset * offset) / (2.0f * dashSigma * dashSigma); // Gaussian formula
    return dashPeakSpeed * expf(exponent);                                      // Scale the speed to peak at dashPeakSpeed
    /* dashPeakSpeed = 12.0f; dashSigma = 0.075f These values can be tweaked to adjust the shape of the speed curve
    scale the curve by 12 so the peak of the dash reaches the desired maximum speed instead of 1 */
}

// Function to attempt to start a dash in the given direction,
// returns true if dash started successfully
static bool startDash(float moveDx, float moveDy)
{
    // Can't start a dash if we're already dashing or if the dash is on cooldown
    if (dashActive || dashCooldownTimer > 0.0f)
    {
        return false;
    }
    // Can't start a dash if thier isnt enough stamina
    if (g.stamina < g.dashStaminaCost)
    {
        return false;
    }
    // If there's movement input, dash in that direction
    // Otherwise, dash in the current facing direction.
    if (moveDx != 0.0f || moveDy != 0.0f)
    {
        dashDirX = moveDx;
        dashDirY = moveDy;
    }
    else
    {
        getDirectionVector(dir, dashDirX, dashDirY);
    }
    // dashActive is set to true to indicate we're now in a dash, and timers are reset
    dashActive = true;
    dashTimer = 0.0f;
    dashCooldownTimer = dashCooldownDuration;
    g.stamina -= g.dashStaminaCost;
    if (g.stamina < 0.0f)
    {
        g.stamina = 0.0f;
    }
    g.staminaRegenTimer = g.staminaRegenDelay;
    return true;
}

// Function to trigger the player's sparkle effect,
// which is a visual effect that plays when the player picks up a gem
void triggerPlayerSparkle()
{
    sparkleTimer = sparkleDuration;
    sparkleAngle = 0.0f;
}

// Function to trigger the player's hurt state,
// which plays a hurt animation and sound, and starts a cooldown before the player can be hurt again
void triggerPlayerHurt()
{
    if (hurtActive || hurtCooldown > 0)
    {
        return;
    }
    hurtActive = true;
    hurtSound.play();
    hurtFrame = 0;
    hurtFrameCounter = 0;
    hurtCooldown = hurtCooldownDuration;
}

// Function to initialize player state and load sprite frames, called at the start of the game
void initPlayer()
{
    playerPosX = g.xres * 0.5f;
    playerPosY = 120.0f;
    speed = baseSpeed;
    sparkleTimer = 0.0f;
    sparkleAngle = 0.0f;
    hurtActive = false;
    hurtFrame = 0;
    hurtFrameCounter = 0;
    hurtCooldown = 0;
    dashActive = false;
    dashKeyHeld = false;
    dashTimer = 0.0f;
    dashCooldownTimer = 0.0f;
    dashDirX = 0.0f;
    dashDirY = -1.0f;

    /* Initialize all sprite frames for the player and hurt states
     calls init_gl() on each Image to load them into OpenGL textures from their respective files */
    for (int i = 0; i < 4; i++)
    {
        southFrames[i].init_gl();
        northFrames[i].init_gl();
        eastFrames[i].init_gl();
        westFrames[i].init_gl();
        neFrames[i].init_gl();
        nwFrames[i].init_gl();
        seFrames[i].init_gl();
        swFrames[i].init_gl();
    }
    // hurt frames
    for (int i = 0; i < hurtFrameCount; i++)
    {
        hurtSouthFrames[i].init_gl();
        hurtNorthFrames[i].init_gl();
        hurtEastFrames[i].init_gl();
        hurtWestFrames[i].init_gl();
        hurtNeFrames[i].init_gl();
        hurtNwFrames[i].init_gl();
        hurtSeFrames[i].init_gl();
        hurtSwFrames[i].init_gl();
    }
}

// Function to update player state each frame, called in the main game loop
void updatePlayer()
{
    const float dt = 1.0f / 60.0f;
    // sparkle timer update
    if (sparkleTimer > 0.0f) // If the sparkle timer is active, then it is updated and the sparkle angle for the visual effect
    {
        sparkleTimer -= dt;
        if (sparkleTimer < 0.0f)
        {
            sparkleTimer = 0.0f;
        }
        sparkleAngle += 8.0f;
    }
    // hurt timer update
    if (hurtActive)
    {
        hurtFrameCounter++;
        if (hurtFrameCounter >= hurtFrameDelay) // Advance hurt animation frame every few updates to control animation speed
        {
            hurtFrameCounter = 0;
            hurtFrame++;
            if (hurtFrame >= hurtFrameCount)
            {
                hurtFrame = 0;
                hurtActive = false;
            }
        }
    }
    if (hurtCooldown > 0) // Decrease hurt cooldown timer, which prevents the player from being hurt again immediately after being hurt
    {
        hurtCooldown--;
    }
    if (dashCooldownTimer > 0.0f) // Decrease dash cooldown timer, which prevents the player from dashing again immediately after a dash
    {
        dashCooldownTimer -= dt;
        if (dashCooldownTimer < 0.0f)
        {
            dashCooldownTimer = 0.0f;
        }
    }
    if (g.staminaRegenTimer > 0.0f) // If the stamina regen timer is active, decrease it
                                    //The player can't regenerate stamina until this timer reaches 0 after dashing
    {
        g.staminaRegenTimer -= dt;
        if (g.staminaRegenTimer < 0.0f)
        {
            g.staminaRegenTimer = 0.0f;
        }
    }
    else if (g.stamina < g.maxStamina)
    {
        g.stamina += g.staminaRegenRate * dt;
        if (g.stamina > g.maxStamina)
        {
            g.stamina = g.maxStamina;
        }
    }

    /* Player movement input handling, we check for WASD or arrow keys and set dx/dy, then normalize the movement vector to prevent 
    faster diagonal movement, We also check for dash input (Shift key) and start a dash if it's pressed 
    and not already dashing or on cooldown. During a dash, the player moves in the dash direction with a 
    speed determined by the Gaussian function for smooth acceleration and deceleration
    */
    float dx = 0.0f;
    float dy = 0.0f;
    bool moving = false;
    if (g_keys[XK_a] || g_keys[XK_Left])
    {
        dx -= 1.0f;
    }

    if (g_keys[XK_d] || g_keys[XK_Right])
    {
        dx += 1.0f;
    }

    if (g_keys[XK_w] || g_keys[XK_Up])
    {
        dy += 1.0f;
    }

    if (g_keys[XK_s] || g_keys[XK_Down])
    {
        dy -= 1.0f;
    }

    if (dx != 0.0f || dy != 0.0f)
    {
        float len = sqrt(dx * dx + dy * dy);
        dx /= len;
        dy /= len;
    }

    // Check for dash input (Shift key) and start dash if conditions are met
    const bool dashPressed = g_keys[XK_Shift_L] || g_keys[XK_Shift_R];
    if (dashPressed && !dashKeyHeld)
    {
        startDash(dx, dy);
    }
    dashKeyHeld = dashPressed;

    if (dashActive)
    {
        moving = true;
        dashTimer += dt;
        playerPosX += dashDirX * getGaussianDashSpeed(dashTimer); // chaning player position based on dash direction and speed from Gaussian function
        playerPosY += dashDirY * getGaussianDashSpeed(dashTimer);
        setDirectionFromMovement(dashDirX, dashDirY);
        if (dashTimer >= dashDuration)
        {
            dashActive = false;
            dashTimer = 0.0f;
        }
    }
    else if (dx != 0.0f || dy != 0.0f)
    {
        moving = true;

        playerPosX += dx * speed; // Update player position based on input direction and base speed
        playerPosY += dy * speed;
        setDirectionFromMovement(dx, dy);
    }

    // Animation frame update, it is only advanced if the player is moving
    // otherwise it is reset to the first frame for an idle pose
    if (moving)
    {
        frameCounter++;

        if (frameCounter > 8)
        {
            currentFrame++;

            if (currentFrame > 3)
            {
                currentFrame = 0;
            }

            frameCounter = 0;
        }
    }
    else
    {
        currentFrame = 0;
    }

    // clamp player inside screen
    const float center = g.xres * 0.5f;
    const float horizontalLimit = 140.0f;

    if (playerPosX < center - horizontalLimit)
    {
        playerPosX = center - horizontalLimit;
    }

    if (playerPosX > center + horizontalLimit)
    {
        playerPosX = center + horizontalLimit;
    }

    if (playerPosY < 0.0f)
    {
        playerPosY = 0.0f;
    }
}

void drawPlayer()
{
    Image *frames;
    Image *hurtFrames;
    // select normal and hurt frames based on direction
    switch (dir)
    {
    case DIR_S:
        frames = southFrames;
        break;
    case DIR_SW:
        frames = swFrames;
        break;
    case DIR_W:
        frames = westFrames;
        break;
    case DIR_NW:
        frames = nwFrames;
        break;
    case DIR_N:
        frames = northFrames;
        break;
    case DIR_NE:
        frames = neFrames;
        break;
    case DIR_E:
        frames = eastFrames;
        break;
    case DIR_SE:
        frames = seFrames;
        break;
    }

    switch (dir)
    {
    case DIR_S:
        hurtFrames = hurtSouthFrames;
        break;
    case DIR_SW:
        hurtFrames = hurtSwFrames;
        break;
    case DIR_W:
        hurtFrames = hurtWestFrames;
        break;
    case DIR_NW:
        hurtFrames = hurtNwFrames;
        break;
    case DIR_N:
        hurtFrames = hurtNorthFrames;
        break;
    case DIR_NE:
        hurtFrames = hurtNeFrames;
        break;
    case DIR_E:
        hurtFrames = hurtEastFrames;
        break;
    case DIR_SE:
        hurtFrames = hurtSeFrames;
        break;
    }

    float screenX = playerPosX - g.cameraX;
    float screenY = playerPosY - g.cameraY;
    bool showPlayer = !(g.hurtTimer > 0 && !hurtActive &&
                        ((g.hurtTimer / 5) % 2 == 0));

    if (showPlayer) // shows player sprite, but blinks when hurt timer is active 
    {
        if (hurtActive)
        {
            hurtFrames[hurtFrame].show(25.0f, (int)screenX, (int)screenY, 0.0f);
        }
        else
        {
            frames[currentFrame].show(25.0f, (int)screenX, (int)screenY, 0.0f);
        }
    }

    /* Sparkle effect rendering, it orbits 4 small diamond sprites around the player when active, 
    with the radius and size of the sparkles changing over time for a dynamic effect
    */
    if (sparkleTimer > 0.0f)
    {
        const float progress = sparkleTimer / sparkleDuration;
        const float orbitRadius = 22.0f + (1.0f - progress) * 8.0f;
        const float sparkleSize = 5.0f + sinf(sparkleAngle * 0.08f) * 1.5f;

        for (int i = 0; i < 4; i++)
        {
            /*https://en.cppreference.com/cpp/numeric/math/sin cos for circular motion 
            of sparkles around the player, angle is offset for each sparkle to space
            them evenly in a circle 
            https://tutorial.math.lamar.edu/classes/calcii/parametriceqn.aspx
            for parametric equations of circle */
            const float angle = sparkleAngle * 0.04f + i * 1.57079632679f;
            const float sparkleX = screenX + cosf(angle) * orbitRadius;
            const float sparkleY = screenY + sinf(angle) * orbitRadius;
            g.diamond.show(
                sparkleSize,
                (int)sparkleX,
                (int)sparkleY,
                sparkleAngle + i * 45.0f,
                0);
        }
    }
}
