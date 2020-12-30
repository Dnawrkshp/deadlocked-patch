/***************************************************
 * FILENAME :		graphics.h
 * 
 * DESCRIPTION :
 * 		
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _LIBDL_GRAPHICS_H_
#define _LIBDL_GRAPHICS_H_

#include <tamtypes.h>
#include "math3d.h"


//--------------------------------------------------------
#define SCREEN_WIDTH           (512)
#define SCREEN_HEIGHT          (448)

//--------------------------------------------------------
typedef float POINT[2] __attribute__((__aligned__(8)));

//--------------------------------------------------------
typedef struct RECT
{
    POINT TopLeft;
    POINT TopRight;
    POINT BottomLeft;
    POINT BottomRight;
} RECT;


/*
 * NAME :		gfxScreenSpaceText
 * 
 * DESCRIPTION :
 * 			Draws the text on the screen.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int gfxScreenSpaceText(int x, int y, float scaleX, float scaleY, u32 color, const char * string, int length);

/*
 * NAME :		gfxScreenSpaceBox
 * 
 * DESCRIPTION :
 * 			Draws a box on the screen.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      rect:           All four corners of box. Use values 0-1 for x and y position.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void gfxScreenSpaceBox(RECT * rect, u32 colorTL, u32 colorTR, u32 colorBL, u32 colorBR);

/*
 * NAME :		gfxWorldSpaceToScreenSpace
 * 
 * DESCRIPTION :
 * 			Converts world space to screen space pixel coordinates.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int gfxWorldSpaceToScreenSpace(VECTOR position, int * x, int * y);


void gfxScreenSpacePIF(RECT * rect);

#endif // _LIBDL_GRAPHICS_H_
