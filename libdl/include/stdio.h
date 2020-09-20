/***************************************************
 * FILENAME :		stdio.h
 * 
 * DESCRIPTION :
 * 		Contains stdio function definitions found in Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _STDIO_H_
#define _STDIO_H_

//--------------------------------------------------------
extern int (*sprintf)(char * buffer, const char * format, ...);

//--------------------------------------------------------
extern int (*printf)(const char * format, ...);


#endif // _STDIO_H_
