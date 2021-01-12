/***************************************************
 * FILENAME :		string.h
 * 
 * DESCRIPTION :
 * 
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _LIBDL_STRING_H_
#define _LIBDL_STRING_H_

//--------------------------------------------------------
inline void * memset(void * ptr, int value, int num);
inline void * memcpy(void * destination, const void * source, int num);
inline char * strncpy(char *dest, const char *src, int n);

#endif // _LIBDL_STRING_H_
