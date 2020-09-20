/***************************************************
 * FILENAME :		sha1.h
 * 
 * DESCRIPTION :
 * 		Contains SHA1 related function definitions found in Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _SHA1_H_
#define _SHA1_H_

/*
 * Computes the SHA1 hash of the input and stores outSize number of bytes of the hash in the output.
 */
int sha1(const void * inBuffer, int inSize, void * outBuffer, int outSize);

#endif // _SHA1_H_

