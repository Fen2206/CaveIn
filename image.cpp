// file: image.cpp
//
// Contains Method Implementations for:
//    Image Class        from Rainforest framework
//    X11_wrapper Class  from asteroids  framework
//
// Some Image class implementations are in other source files   

#include "image.h"
//#include "shared.h"
#include <cstring>
#include <iostream>
#include <unistd.h>


#define SAFE_LINE_READ(line, count, pointer) \
    if ( fgets(line, count, pointer) == NULL)\
        std::cout << "error reading line " << line << " of image file\n"
// Semicolon must be used during implementation of this macro
// Writen this way follow style

// =============================================================
// Extracted or taken whole from rainforest framework by David.
// Rainforest start:
Image::Image(const char *fname)
{ // From the rainforest framework
    // ========================================================
    // Analyse fname string to set up convert command
    if (fname[0] == '\0')
        return;
    //printf("fname **%s**\n", fname);
    char name[40];
    strcpy(name, fname);
    const int slen = strlen(name);
    const int ppmFlag = strncmp(name+(slen-4), ".ppm", 4) == 0;
    is_jpg = strncmp(name+(slen-4), ".jpg", 4) == 0;
    const unsigned char clr_byte = is_jpg ? 255 : 0;
    for (int i=0; i<3; i++)
        alphaColor[i] = clr_byte;
    //jpg == white, everything else == black
    char ppmname[80];
    // ========================================================
    // Uses CLI MagickConvert to get PPM
    if (ppmFlag) {
        strcpy(ppmname, name);
    } else {
        name[slen-4] = '\0';
        sprintf(ppmname,"%s.ppm", name);
        char convert_command[100];
        //system("convert eball.jpg eball.ppm");
        sprintf(convert_command, "convert %s %s", fname, ppmname);
        if( system(convert_command) )
            std::cout << "Magick Convert failed\n";
    }
    FILE *fpi = fopen(ppmname, "r");
    if (fpi) {
        char line[200];
        //First two lines of ppm must be trashed
        SAFE_LINE_READ(line, 200, fpi);
        SAFE_LINE_READ(line, 200, fpi);
        //skip comments and blank lines
        while (line[0] == '#' || strlen(line) < 2)
            SAFE_LINE_READ(line, 200, fpi);
        //Width and Height extracted according to ppm encoding
        sscanf(line, "%i %i", &width, &height);
        SAFE_LINE_READ(line, 200, fpi);
        //get pixel data
        const int n = width * height * 3;
        data = new unsigned char[n];
        for (int i=0; i<n; i++)
            data[i] = fgetc(fpi);
        //printf("%s", data);
        fclose(fpi);
    } else {
        printf("ERROR opening image: %s\n",ppmname);
        exit(0);
    }
    // Delete the temporary ppm file, but only if we used convert to make it
    if (!ppmFlag)
        unlink(ppmname);
} 

void Image::show(float wid, int pos_x, int pos_y, float angle, int flipped)
{
    Image* img = this;
    const float height = wid * img->height/img->width;
    glBindTexture(GL_TEXTURE_2D, img->texture);
    glColor4f(0.0, 0.0, 0.0, 0.0);
    glPushMatrix(); // Pushmatrix indented because subequent commands
                    // Are implemented on the topmost stack as determined
                    // By glPush and glPop
        glTranslatef(pos_x, pos_y, 0.0f);
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.0f);
        glColor4ub(255,255,255,255);
        glRotatef(angle, 0.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
        if (flipped) {
            glTexCoord2i(1, 1); glVertex2i(-wid,-height);
            glTexCoord2i(1, 0); glVertex2i(-wid, height);
            glTexCoord2i(0, 0); glVertex2i( wid, height);
            glTexCoord2i(0, 1); glVertex2i( wid,-height);
        } else {
            //glTexCoord2f(1.0f, 1.0f); glVertex2i(-wid,-height);
            glTexCoord2i(0, 1); glVertex2i(-wid,-height);
            glTexCoord2i(0, 0); glVertex2i(-wid, height);
            glTexCoord2i(1, 0); glVertex2i( wid, height);
            glTexCoord2i(1, 1); glVertex2i( wid,-height);
        }
        glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0); // Unbinds texture
}

void Image::init_gl()
{
    glGenTextures(1, &texture);
    const int w = width;
    const int h = height;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    unsigned char *silhouetteData;
    bool is_black = !(alphaColor[0]||alphaColor[1]||alphaColor[2]);
    if (is_black)
        silhouetteData = this->blackToAlpha();
    else
        silhouetteData = this->colorToAlpha(alphaColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
    free(silhouetteData);
}
unsigned char* Image::blackToAlpha()
{
    unsigned char black[3] = {0, 0, 0};
    return colorToAlpha(black);
}

unsigned char* Image::colorToAlpha(unsigned char alphaColorIn[3])
{
    // Convert RGB data -> RGBA data and make alphaColor transparent.
    // Your loader allocates data as width*height*3 (RGB), so we expand to RGBA.
    if (!data || width <= 0 || height <= 0)
        return NULL;

    const int nPixels = width * height;
    unsigned char *newdata = (unsigned char *)malloc(nPixels * 4);
    if (!newdata) return NULL;

    for (int i = 0; i < nPixels; i++) {
        int j3 = i * 3;
        int j4 = i * 4;

        unsigned char r = data[j3 + 0];
        unsigned char g = data[j3 + 1];
        unsigned char b = data[j3 + 2];

        newdata[j4 + 0] = r;
        newdata[j4 + 1] = g;
        newdata[j4 + 2] = b;

        if (r == alphaColorIn[0] && g == alphaColorIn[1] && b == alphaColorIn[2])
            newdata[j4 + 3] = 0;     // transparent
        else
            newdata[j4 + 3] = 255;   // opaque
    }

    return newdata;
}

// 4-arg overload required by image.h (your asteroids.cpp calls this)
void Image::show(float wid, int pos_x, int pos_y, float angle)
{
    show(wid, pos_x, pos_y, angle, 0);
}

// rainforest end

// =============================================================
