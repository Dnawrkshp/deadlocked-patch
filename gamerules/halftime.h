#ifndef _GAMERULES_HALFTIME_
#define _GAMERULES_HALFTIME_

#include "module.h"

/*
 * NAME :		htReset
 * 
 * DESCRIPTION :
 * 			Resets the state.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void htReset(void);

/*
 * NAME :		halftimeLogic
 * 
 * DESCRIPTION :
 * 			Checks if half the game has passed, and then flips the sides if possible.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void halftimeLogic(GameModule * module);

#if DEBUG

void htCtfBegin(void);
extern int HalfTimeState;

#endif


#endif // _GAMERULES_HALFTIME_
