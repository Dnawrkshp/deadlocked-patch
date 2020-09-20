/***************************************************
 * FILENAME :		string.h
 * 
 * DESCRIPTION :
 * 		Contains many libc string function definitions found in Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _STRING_H_
#define _STRING_H_

//--------------------------------------------------------
void * memcpy(void * dest, const void * src, unsigned int size);

//--------------------------------------------------------
void * memset(void * ptr, int c, unsigned int size);

//--------------------------------------------------------
void * memcmp(const void * p, const void * q, unsigned int size);

//--------------------------------------------------------
int sprintf(char * buffer, const char * format, ...);

//--------------------------------------------------------
int printf(const char * format, ...);

//--------------------------------------------------------
unsigned int strlen(const char * s);


#endif // _STRING_H_
