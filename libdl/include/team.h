/***************************************************
 * FILENAME :		team.h
 * 
 * DESCRIPTION :
 * 		Contains team specific offsets and structures for Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _TEAM_H_
#define _TEAM_H_

#include <tamtypes.h>

/*
 *
 */
enum TEAM_IDS
{
    TEAM_BLUE,
    TEAM_RED,
    TEAM_GREEN,
    TEAM_ORANGE,
    TEAM_YELLOW,
    TEAM_PURPLE,
    TEAM_AQUA,
    TEAM_PINK,
    TEAM_OLIVE,
    TEAM_MAROON,
    TEAM_WHITE
};

extern const u32 TEAM_COLORS[];

#endif // _TEAM_H_
