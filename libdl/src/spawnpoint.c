#include <tamtypes.h>
#include "string.h"
#include "spawnpoint.h"

/*
 * Player 1's hud.
 */
#define SPAWNPOINTS		    ((SpawnPoint*)(*(u32*)0x00222820))

/*
 *
 */
#define SP_COUNT    (*(int*)0x00222824)

/*
 * Player 2's hud.
 */
#define SP_HILL_PTR	        (*(u32*)0x00222698)

/*
 * NAME :		getSpawnPointCount
 * 
 * DESCRIPTION :
 * 			
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int getSpawnPointCount()
{
    return SP_COUNT;
}

/*
 * NAME :		getHillPointCount
 * 
 * DESCRIPTION :
 * 			
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
short getHillPointCount()
{
    u32 * hillInfo = (u32*)SP_HILL_PTR;
    if (!hillInfo)
        return 0;

    return *((short*)(hillInfo + 12) + 1);
}

/*
 * NAME :		setHillPointCount
 * 
 * DESCRIPTION :
 * 			
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setHillPointCount(short count)
{
    u32 * hillInfo = (u32*)SP_HILL_PTR;
    if (!hillInfo)
        return;

    *(short*)(hillInfo + 0x30) = count;
}

/*
 * NAME :		getHillPointIndex
 * 
 * DESCRIPTION :
 * 			
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int getHillPointIndex(int id)
{
    u32 * hillInfo = (u32*)SP_HILL_PTR;
    if (!hillInfo)
        return -1;

    return (hillInfo + 0x50)[id];
}

/*
 * NAME :		setHillPointIndex
 * 
 * DESCRIPTION :
 * 			
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setHillPointIndex(int id, int value)
{
    u32 * hillInfo = (u32*)SP_HILL_PTR;
    if (!hillInfo)
        return;

    (hillInfo + 0x50)[id] = value;
}

/*
 * NAME :		getSpawnPoint
 * 
 * DESCRIPTION :
 * 			
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
SpawnPoint * getSpawnPoint(int index)
{
    SpawnPoint * spawnPoints = SPAWNPOINTS;
    if (!spawnPoints)
        return NULL;

    return &spawnPoints[index];
}

/*
 * NAME :		setSpawnPoint
 * 
 * DESCRIPTION :
 * 			
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setSpawnPoint(SpawnPoint * sp, int index)
{
    SpawnPoint * spawnPoints = SPAWNPOINTS;
    if (!spawnPoints)
        return;

    memcpy(&spawnPoints[index], sp, sizeof(SpawnPoint));
}
