/***************************************************
 * FILENAME :		pvars.h
 * 
 * DESCRIPTION :
 * 		Contains pvar data structure definitions for some relevant mobys.
 * 
 * NOTES :
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */


#ifndef _SND_PVARS_H_
#define _SND_PVARS_H_

#include <tamtypes.h>
#include <libdl/moby.h>

/*
 * NAME :		HackerOrbPVar
 * 
 * DESCRIPTION :
 * 			Incomplete pvar structure for hacker orb moby.
 * 
 * NOTES :
 * 
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
typedef struct HackerOrbPVar
{
    u8 UNK_00[0x04];
    int NodeTeam;
    u8 UNK_08[0x04];
    u32 NodeBaseUid;
} HackerOrbPVar_t;

/*
 * NAME :		NodeBasePVar
 * 
 * DESCRIPTION :
 * 			Incomplete pvar structure for node base moby.
 * 
 * NOTES :
 * 
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
typedef struct NodeBasePVar
{
    u8 UNK_00[8];
    float CaptureTime;
    Moby * HackerOrbMoby;
    u8 UNK_10[0x10];
    Moby* ChildMobies[4];
} NodeBasePVar_t;

#endif // _SND_PVARS_H_
