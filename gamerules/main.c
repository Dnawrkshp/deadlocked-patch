/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Gamerules entrypoint and logic.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#include <tamtypes.h>

#include "common.h"
#include "time.h"
#include "module.h"
#include "game.h"
#include "gamesettings.h"
#include "player.h"
#include "cheats.h"

/*
 * Gamerule ids.
 * Each id is mapped to a single bit in a 32-bit integer.
 */
enum GameRuleIdBitMask
{
	GAMERULE_NONE = 			(0),
	GAMERULE_NO_PACKS =			(1 << 0),
	GAMERULE_NO_V2S =			(1 << 1),
	GAMERULE_MIRROR =			(1 << 2)

};

/*
 * NAME :		gameStart
 * 
 * DESCRIPTION :
 * 			Gamerules game logic entrypoint.
 * 
 * NOTES :
 * 			This is called only when in game.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void gameStart(u32 * arg0)
{
	u32 bitmask = *arg0;

	// If no game rules then exit
	if (bitmask == GAMERULE_NONE)
		return;

	if (bitmask & GAMERULE_NO_PACKS)
		cheatsApplyNoPacks();

	if (bitmask & GAMERULE_NO_V2S)
		cheatsApplyNoV2s();

	if (bitmask & GAMERULE_MIRROR)
		cheatsApplyMirrorWorld(1);
}

/*
 * NAME :		lobbyStart
 * 
 * DESCRIPTION :
 * 			Gamerules lobby logic entrypoint.
 * 
 * NOTES :
 * 			This is called only when in lobby.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void lobbyStart(u32 * arg0)
{
	// If we're not in staging then reset
	if (GLOBAL_GAMESETTINGS)
		return;

	// Reset
	*arg0 = 0;

	// Reset mirror world in lobby
	cheatsApplyMirrorWorld(0);
}
