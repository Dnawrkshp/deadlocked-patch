/***************************************************
 * FILENAME :		common.h
 * 
 * DESCRIPTION :
 * 		Contains many common libc (and more) function definitions found in Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _COMMON_H_
#define _COMMON_H_

/*
 *
 */
void memcpy(void *, const void *, int);

/*
 *
 */
void * memset(void *, int, int);

/*
 *
 */
int memcmp(void * a, void * b, int n);

/*
 * 
 */
extern int (*sprintf)(char *, const char *, ...);

/*
 * 
 */
extern unsigned int (*strlen)(const char *);

/*
 *
 */
extern int (*printf)(const char * format, ...);

/*
 * Computes the SHA1 hash of the input and stores outSize number of bytes of the hash in the output.
 */
extern int (*sha1)(const void * inBuffer, int inSize, void * outBuffer, int outSize);


#endif // _COMMON_H_
