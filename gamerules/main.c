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
#include <string.h>

#include "time.h"
#include "module.h"
#include "game.h"
#include "gamesettings.h"
#include "map.h"
#include "player.h"
#include "cheats.h"
#include "stdio.h"
#include "pad.h"
#include "dl.h"
#include "spawnpoint.h"
#include "halftime.h"
#include "graphics.h"

/*
 * Gamerule ids.
 * Each id is mapped to a single bit in a 32-bit integer.
 */
enum GameRuleIdBitMask
{
	GAMERULE_NONE = 			(0),
	GAMERULE_NO_PACKS =			(1 << 0),
	GAMERULE_NO_V2S =			(1 << 1),
	GAMERULE_MIRROR =			(1 << 2),
	GAMERULE_NO_HB =			(1 << 3),
	GAMERULE_VAMPIRE =			(1 << 4),
	GAMERULE_HALFTIME =			(1 << 5),
	GAMERULE_BETTERHILLS = 		(1 << 6),
	GAMERULE_HEALTHBARS = 		(1 << 7)
};

/*
 *
 */
int Initialized = 0;

/*
 *
 */
int HasDisabledHealthboxes = 0;

/*
 * 
 */
short PlayerKills[GAME_MAX_PLAYERS];

/*
 * Custom hill spawn points
 */
SpawnPoint BetterHillPoints[] = {
	// TORVAL ID 0x13
	{ { 20, 0, 0, 0, 0, 19.907925, -1.916915, 0, 0, 0.9584575, 9.953962, 0, 300, 371, 107, 0 }, { 0.05, 0, 0, 0, 0, 0.04976981, -0.0047922875, 0, 0, 0.009584575, 0.09953962, 0, 300, 371, 107, 0 } },

	// MARAXUS ID 0x13
	{ { 10, 0, 0, 0, 0, 10, 0, 0, 0, 0, 10, 0, 525.83344, 742.3953, 103.92746, 1 }, { 0.1, 0, 0, 0, 0, 0.1, 0, 0, 0, 0, 0.1, 0, 525.83344, 742.3953, 103.92746, 1 } },

	// SARATHOS ID 0x11
	{ { 12, 0, 0, 0, 0, 12, 0, 0, 0, 0, 12, 0, 493.81552, 218.55286, 105.77197, 1 }, { 0.083333336, 0, 0, 0, 0, 0.083333336, 0, 0, 0, 0, 0.083333336, 0, 493.81552, 218.55286, 105.77197, 1 } },

	// CATACROM ID 0x0D
	{ { 7, 0, 0, 0, 0, 7, 0, 0, 0, 0, 7, 0, 333.3385, 255.27766, 63.045467, 0 }, { 0.14285715, 0, 0, 0, 0, 0.14285715, 0, 0, 0, 0, 0.14285715, 0, 333.3385, 255.27766, 63.045467, 0 } },
	
	// CATACROM ID 0x11
	{ { 7, 0, 0, 0, 0, 7, 0, 0, 0, 0, 7, 0, 324.36548, 366.6925, 65.79945, 0 }, { 0.14285715, 0, 0, 0, 0, 0.14285715, 0, 0, 0, 0, 0.14285715, 0, 324.36548, 366.6925, 65.79945, 0 } },

	// DC ID 0x14
	{ { 12, 0, 0, 0, 0, 12, 0, 0, 0, 0, 12, 0, 503.789, 494.31857, 641.2969, 1 }, { 0.083333336, 0, 0, 0, 0, 0.083333336, 0, 0, 0, 0, 0.083333336, 0, 503.789, 494.31857, 641.2969, 1 } },

	// DC ID 0x16
	{ { 16, 0, 0, 0, 0, 16, 0, 0, 0, 0, 16, 0, 349.3588, 529.6109, 641.25, 1 }, { 0.0625, 0, 0, 0, 0, 0.0625, 0, 0, 0, 0, 0.0625, 0, 349.3588, 529.6109, 641.25, 1 } },

	// SHAAR ID 0x20
	{ { 12, 0, 0, 0, 0, 12, 0, 0, 0, 0, 12, 0, 621.2297, 562.5671, 509.29688, 0 }, { 0.083333336, 0, 0, 0, 0, 0.083333336, 0, 0, 0, 0, 0.083333336, 0, 621.2297, 562.5671, 509.29688, 0 } },

	// SHAAR ID 0x23
	{ { 10, 0, 0, 0, 0, 10, 0, 0, 0, 0, 10, 0, 487.00687, 682.48615, 509.3125, 0 }, { 0.1, 0, 0, 0, 0, 0.1, 0, 0, 0, 0, 0.1, 0, 487.00687, 682.48615, 509.3125, 0 } },

	// GC ID 0x20
	{ { 7, 0, 0, 0, 0, 7, 0, 0, 0, 0, 7, 0, 636.95593, 401.78497, 102.765625, 0 }, { 0.14285715, 0, 0, 0, 0, 0.14285715, 0, 0, 0, 0, 0.14285715, 0, 636.95593, 401.78497, 102.765625, 0 } },

	// GC ID 0x21
	{ { 12, 0, 0, 0, 0, 12, 0, 0, 0, 0, 12, 0, 728.6185, 563.659, 101.578125, 1 }, { 0.083333336, 0, 0, 0, 0, 0.083333336, 0, 0, 0, 0, 0.083333336, 0, 728.6185, 563.659, 101.578125, 1 } },

	// GC ID 0x23
	{ { 6.7093644, 4.3571124, 0, 0, -4.9017515, 7.548035, 0, 0, 0, 0, 8, 0, 586.91223, 541.42474, 102.828125, 0 }, { 0.10483382, 0.06807988, 0, 0, -0.060515452, 0.09318562, 0, 0, 0, 0, 0.125, 0, 586.91223, 541.42474, 102.828125, 0 } },


};

enum BETTER_HILL_PTS
{
	TORVAL_13 = 0,
	MARAXUS_13 = 1,
	SARATHOS_11 = 2,
	CATACROM_0D = 3,
	CATACROM_11 = 4,
	DC_14 = 5,
	DC_16 = 6,
	SHAAR_14 = 7,
	SHAAR_17 = 8,
	GS_20 = 9,
	GS_21 = 10,
	GS_22 = 11,


};

/*
 * NAME :		vampireLogic
 * 
 * DESCRIPTION :
 * 			Checks if a player has gotten a kill and heals them if so.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void vampireLogic(GameModule * module)
{
	int i;
	Player ** playerObjects = getPlayers();
	Player * player;
	PlayerGameStats * stats = getPlayerGameStats();

	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		// Check if player has killed someone
		if (stats->Kills[i] > PlayerKills[i])
		{
			// Try to heal if player exists
			player = playerObjects[i];
			if (player)
				player->Health = PLAYER_MAX_HEALTH;
			
			// Update our cached kills count
			PlayerKills[i] = stats->Kills[i];
		}
	}
}

int betterHillsInitialized = 0;

/*
 * NAME :		betterHillsLogic
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
void betterHillsLogic(GameModule * module)
{
	if (betterHillsInitialized)
		return;

	//
	betterHillsInitialized = 1;

	// 
	switch (getGameSettings()->GameLevel)
	{
		case MAP_ID_CATACROM:
		{
			setSpawnPoint(&BetterHillPoints[CATACROM_0D], 0x0D);
			setSpawnPoint(&BetterHillPoints[CATACROM_11], 0x11);
			break;
		}
		case MAP_ID_SARATHOS:
		{
			setSpawnPoint(&BetterHillPoints[SARATHOS_11], 0x11);
			break;
		}
		case MAP_ID_DC:
		{
			setSpawnPoint(&BetterHillPoints[DC_14], 0x0E);
			setSpawnPoint(&BetterHillPoints[DC_16], 0x10);
			break;
		}
		case MAP_ID_SHAAR: 
		{
			setSpawnPoint(&BetterHillPoints[SHAAR_14], 0x14);
			setSpawnPoint(&BetterHillPoints[SHAAR_17], 0x17);
			break;
		}
		case MAP_ID_MARAXUS: 
		{
			setSpawnPoint(&BetterHillPoints[MARAXUS_13], 0x13);
			break;
		}
		case MAP_ID_GS:
		{
			setSpawnPoint(&BetterHillPoints[GS_20], 0x14);
			setSpawnPoint(&BetterHillPoints[GS_21], 0x15);
			setSpawnPoint(&BetterHillPoints[GS_22], 0x16);
			break;
		}
		case MAP_ID_TORVAL:
		{
			setSpawnPoint(&BetterHillPoints[TORVAL_13], 0x13);
			break;
		}
	}
}

/*
 * NAME :		healthbarsLogic
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
void healthbarsLogic(GameModule * module)
{
	float distanceScale = 0;

	if (isInGame())
	{
		Player ** players = getPlayers();
		Player * localPlayer = (Player*)0x00347AA0;
		int px, py, i;

		for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		{
			Player * player = players[i];
			VECTOR temp;
			if (!player || isLocal(player) || player->Health <= 0)
				continue;

			// Check distance
			vector_subtract(temp, player->PlayerPosition, localPlayer->PlayerPosition);
			float distance = vector_length(temp);
			if (distance > 75)
				continue;
			
			// Check angle
			if (vector_innerproduct(temp, (float*)0x0022CD20) < 0)
				continue;

			// get 0-1 based on distance
			distanceScale = clamp((75 - distance) / 75, 0, 1);

			// move position above player
			vector_copy(temp, player->PlayerPosition);
			temp[2] += distanceScale * 1.05;

			if (gfxWorldSpaceToScreenSpace(temp, &px, &py))
			{
				py -= powf(distanceScale, 4) * 27;
				float x = (float)px / SCREEN_WIDTH;
				float y = (float)py / SCREEN_HEIGHT;
				float health = player->Health / 50;
				float w = (0.05 * distanceScale) + 0.02, h = 0.005, p = 0.002;
				float right = w * health * 2;
				u32 color = TEAM_COLORS[player->Team];

				// Draw boxes
				RECT r = { {x-w-p,y-h-p}, {x+w+p,y-h-p}, {x-w-p,y+h+p}, {x+w+p,y+h+p} };
				gfxScreenSpaceBox(&r, 0x80000000, 0x80000000, 0x80000000, 0x80000000);
				RECT r2 = { {x-w,y-h}, {x-w +right,y-h}, {x-w,y+h}, {x-w + right,y+h} };
				gfxScreenSpaceBox(&r2, color, color, color, color);
			}
		}

	}
}

/*
 * NAME :		initialize
 * 
 * DESCRIPTION :
 * 			Initializes this module.
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
void initialize(void)
{
	int i;

	// Initialize player kills to 0
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		PlayerKills[i] = 0;

	// reset
	htReset();
	betterHillsInitialized = 0;

	Initialized = 1;
}

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
void gameStart(GameModule * module)
{
	u32 bitmask = *(u32*)module->Argbuffer;
	char weatherId = module->Argbuffer[4];

	// Initialize
	if (Initialized != 1)
		initialize();

	// Apply weather
	cheatsApplyWeather(weatherId);

#if DEBUG
	dlPreUpdate();

	halftimeLogic(module);
	if (padGetButtonDown(0, PAD_L3 | PAD_R3) > 0)
	{
		htCtfBegin();
		HalfTimeState = 1;
	}

	dlPostUpdate();
#endif

	// If no game rules then exit
	if (bitmask == GAMERULE_NONE)
		return;

	if (bitmask & GAMERULE_NO_PACKS)
		cheatsApplyNoPacks();

	if (bitmask & GAMERULE_NO_V2S)
		cheatsApplyNoV2s();

	if (bitmask & GAMERULE_MIRROR)
		cheatsApplyMirrorWorld(1);

	if (bitmask & GAMERULE_NO_HB && !HasDisabledHealthboxes)
		HasDisabledHealthboxes = cheatsDisableHealthboxes();

	if (bitmask & GAMERULE_VAMPIRE)
		vampireLogic(module);

	if (bitmask & GAMERULE_HALFTIME)
		halftimeLogic(module);

	if (bitmask & GAMERULE_BETTERHILLS)
		betterHillsLogic(module);

	if (bitmask & GAMERULE_HEALTHBARS)
		healthbarsLogic(module);
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
void lobbyStart(GameModule * module)
{
	GameSettings * gameSettings = getGameSettings();

	// If we're not in staging then reset
	if (!gameSettings)
		return;

	// Check we're not loading
	if (gameSettings->GameLoadStartTime > 0)
		return;

	// Reset
	memset(module->Argbuffer, 0, GAME_MODULE_ARG_SIZE);
	module->State = GAMEMODULE_OFF;
	Initialized = 0;

	// Reset mirror world in lobby
	cheatsApplyMirrorWorld(0);
}
