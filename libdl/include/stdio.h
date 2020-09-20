/***************************************************
 * FILENAME :		stdio.h
 * 
 * DESCRIPTION :
 * 		Contains useful functions for debugging.
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _GAME_STDIO_H_
#define _GAME_STDIO_H_

//--------------------------------------------------------
extern int (*sprintf)(char * buffer, const char * format, ...);

//--------------------------------------------------------
extern int (*printf)(const char * format, ...);

#endif // _GAME_STDIO_H_
