/***************************************************
 * FILENAME :		map.h
 * 
 * DESCRIPTION :
 * 		Contains map specific offsets and structures for Deadlocked.
 * 
 * NOTES :
 * 
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _LIBDL_MAP_H_
#define _LIBDL_MAP_H_

#include <tamtypes.h>

/*
 *
 */
enum MAP_IDS
{
    MAP_ID_BATTLEDOME   = 41,
    MAP_ID_CATACROM     = 42,
    MAP_ID_SARATHOS     = 44,
    MAP_ID_DC           = 45,
    MAP_ID_SHAAR        = 46,
    MAP_ID_VALIX        = 47,
    MAP_ID_MF           = 48,
    MAP_ID_TORVAL       = 50,
    MAP_ID_TEMPUS       = 51,
    MAP_ID_MARAXUS      = 53,
    MAP_ID_GS           = 54
};

enum MAP_MASK_IDS
{
    MAP_MASK_NONE         = 0,
    MAP_MASK_BATTLEDOME   = (1 << 0),
    MAP_MASK_CATACROM     = (1 << 1),
    MAP_MASK_SARATHOS     = (1 << 2),
    MAP_MASK_DC           = (1 << 3),
    MAP_MASK_SHAAR        = (1 << 4),
    MAP_MASK_VALIX        = (1 << 5),
    MAP_MASK_MF           = (1 << 6),
    MAP_MASK_TORVAL       = (1 << 7),
    MAP_MASK_TEMPUS       = (1 << 8),
    MAP_MASK_MARAXUS      = (1 << 9),
    MAP_MASK_GS           = (1 << 10),
    MAP_MASK_ALL          = -1
};

int mapMaskHasMask(int value, int maskValue);
int mapIdToMask(int mapId);
int mapMaskToId(int mapMask);

#endif // _LIBDL_MAP_H_
