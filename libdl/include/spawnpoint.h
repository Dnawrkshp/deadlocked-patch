/***************************************************
 * FILENAME :		spawnpoint.h
 * 
 * DESCRIPTION :
 * 		Contains spawnpoint specific offsets and structures for Deadlocked.
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _LIBDL_SPAWNPOINT_H_
#define _LIBDL_SPAWNPOINT_H_

#include "math3d.h"

/*
 * NAME :		SpawnPoint
 * 
 * DESCRIPTION :
 * 			
 * 
 * NOTES :
 * 
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
typedef struct SpawnPoint
{
    MATRIX M0;
    MATRIX M1;
} SpawnPoint;

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
int getSpawnPointCount();

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
short getHillPointCount();

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
void setHillPointCount(short count);

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
int getHillPointIndex(int id);

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
void setHillPointIndex(int id, int value);

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
SpawnPoint * getSpawnPoint(int index);

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
void setSpawnPoint(SpawnPoint * sp, int index);

#endif // _LIBDL_SPAWNPOINT_H_
