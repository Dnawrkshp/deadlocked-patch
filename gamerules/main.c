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
	GAMERULE_BETTERHILLS = 		(1 << 6)
};

enum HalfTimeStates
{
	HT_WAITING,
	HT_INTERMISSION,
	HT_SWITCH,
	HT_INTERMISSION2,
	HT_COMPLETED
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
int HalfTimeState = 0;

/*
 * Indicates when the half time grace period should end.
 */
int HalfTimeEnd = -1;

/*
 * Flag moby by team id.
 */
Moby * CtfFlags[4] = {0,0,0,0};

/*
 * 
 */
short PlayerKills[GAME_MAX_PLAYERS];

/*
 * When non-zero, it refreshes the in-game scoreboard.
 */
#define GAME_SCOREBOARD_REFRESH_FLAG        (*(u32*)0x00310248)

/*
 * Target scoreboard value.
 */
#define GAME_SCOREBOARD_TARGET              (*(u32*)0x002FA084)

/*
 * Collection of scoreboard items.
 */
#define GAME_SCOREBOARD_ARRAY               ((ScoreboardItem**)0x002FA04C)

/*
 * Number of items in the scoreboard.
 */
#define GAME_SCOREBOARD_ITEM_COUNT          (*(u32*)0x002F9FCC)

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
 * NAME :		getFlags
 * 
 * DESCRIPTION :
 * 			Grabs all four flags and stores their moby pointers in CtfFlags.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void getFlags(void)
{
	Moby ** mobies = getLoadedMobies();
	Moby * moby;

	// reset
	CtfFlags[0] = CtfFlags[1] = CtfFlags[2] = CtfFlags[3] = 0;

	// grab flags
	while ((moby = *mobies))
	{
		if (moby->MobyId == MOBY_ID_BLUE_FLAG ||
			moby->MobyId == MOBY_ID_RED_FLAG ||
			moby->MobyId == MOBY_ID_GREEN_FLAG ||
			moby->MobyId == MOBY_ID_ORANGE_FLAG)
		{
			CtfFlags[*(u16*)(moby->PropertiesPointer + 0x14)] = moby;
		}

		++mobies;
	}
}

/*
 * NAME :		htCtfBegin
 * 
 * DESCRIPTION :
 * 			Performs all the operations needed at the start of half time.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void htCtfBegin(void)
{
	// Make sure we have the flag mobies
	getFlags();

	// Indicate when the intermission should end
	HalfTimeEnd = getGameTime() + (TIME_SECOND * 5);
	HalfTimeState = HT_INTERMISSION;

	// Disable saving or pickup up flag
	flagSetPickupDistance(0);

	// Show popup
	showPopup(0, "Halftime");
	showPopup(1, "Halftime");
}


/*
 * NAME :		htCtfSwitch
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
void htCtfSwitch(void)
{
	int i, j;
	Player ** players = getPlayers();
	Player * player;
	Moby * moby;
	ScoreboardItem * scoreboardItem;
	VECTOR rVector, pVector;
	u8 * teamCaps = getTeamStatCaps();
	int teams = 0;
	u8 teamChangeMap[4] = {0,1,2,3};
	u8 backupTeamCaps[4];
	int scoreboardItemCount = GAME_SCOREBOARD_ITEM_COUNT;

	// 
	memset(rVector, 0, sizeof(VECTOR));

	// backup
	memcpy(backupTeamCaps, teamCaps, 4);
	
	// Determine teams
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		player = players[i];
		if (!player)
			continue;
			
		teams |= (1 << (player->Team+1));
	}

	// If all four teams then just swap
	if (teams == 0xF)
	{
		teamChangeMap[TEAM_BLUE] = TEAM_RED;
		teamChangeMap[TEAM_RED] = TEAM_BLUE;
		teamChangeMap[TEAM_GREEN] = TEAM_ORANGE;
		teamChangeMap[TEAM_ORANGE] = TEAM_GREEN;
	}
	// Otherwise rotate the teams
	else
	{
		for (i = 0; i < 4; ++i)
		{
			if (!(teams & (1 << (i+1))))
				continue;

			j = i;
			do
			{
				++j;
				if (j >= 4)
					j = 0;

				if (!(teams & (1 << (j+1))))
					continue;

				teamChangeMap[i] = j;
				break;
			} while (j != i);
		}
	}

	// Switch player teams
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		player = players[i];
		if (!player)
			continue;

		if (isLocal(player))
		{
			// Update local scoreboard
			for (j = 0; j < scoreboardItemCount; ++j)
			{
				scoreboardItem = GAME_SCOREBOARD_ARRAY[j];
				if (!scoreboardItem)
					continue;

				// Swap
				if (scoreboardItem->TeamId == teamChangeMap[player->Team])
				{
					ScoreboardItem * temp = GAME_SCOREBOARD_ARRAY[player->LocalPlayerIndex];
					GAME_SCOREBOARD_ARRAY[player->LocalPlayerIndex] = scoreboardItem;
					GAME_SCOREBOARD_ARRAY[j] = temp;
					break;
				}
			}
		}

		// Kick from vehicle
		if (player->Vehicle)
			VehicleRemovePlayer(player->Vehicle, player);

		// Change to new team
		changeTeam(player, teamChangeMap[player->Team]);

		// Respawn
		playerRespawn(player);

		// Teleport player to base
		moby = CtfFlags[player->Team];
		if (moby)
		{
			vector_copy(pVector, (float*)moby->PropertiesPointer);
			float theta = (player->PlayerId / (float)GAME_MAX_PLAYERS) * MATH_TAU;
			pVector[0] += cosf(theta) * 2.5;
			pVector[1] += sinf(theta) * 2.5;
			playerSetPosRot(player, pVector, rVector);
		}
	}

	// Switch team scores
	for (i = 0; i < 4; ++i)
		teamCaps[i] = backupTeamCaps[teamChangeMap[i]];

	// reset flags
	for (i = 0; i < 4; ++i)
	{
		if (CtfFlags[i])
		{
			*(u16*)(CtfFlags[i]->PropertiesPointer + 0x10) = 0xFFFF;
			vector_copy((float*)&CtfFlags[i]->Position, (float*)CtfFlags[i]->PropertiesPointer);
		}
	}
}

/*
 * NAME :		htCtfEnd
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
void htCtfEnd(void)
{
	// Enable flag pickup
	flagSetPickupDistance(2);
}


/*
 * NAME :		htCtfTick
 * 
 * DESCRIPTION :
 * 			Performs all the operations needed while in intermission.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void htCtfTick(void)
{
	int i;
	Player ** players = getPlayers();
	int gameTime = getGameTime();

	// Prevent input
	padResetInput(0);
	padResetInput(1);

	switch (HalfTimeState)
	{
		case HT_INTERMISSION:
		{
			if (gameTime > (HalfTimeEnd - (TIME_SECOND * 2)))
				HalfTimeState = HT_SWITCH;
			break;
		}
		case HT_SWITCH:
		{
			// Show popup
			if (gameTime > (HalfTimeEnd - (TIME_SECOND * 2)))
			{
				showPopup(0, "switching sides...");
				showPopup(1, "switching sides...");

				htCtfSwitch();
				HalfTimeState = HT_INTERMISSION2;
			}
			break;
		}
		case HT_INTERMISSION2:
		{
			// Drop held items
			for (i = 0; i < GAME_MAX_PLAYERS; ++i)
			{
				if (!players[i])
					continue;

				players[i]->HeldMoby = 0;
			}

			// reset flags
			for (i = 0; i < 4; ++i)
			{
				if (CtfFlags[i])
				{
					*(u16*)(CtfFlags[i]->PropertiesPointer + 0x10) = 0xFFFF;
					vector_copy((float*)&CtfFlags[i]->Position, (float*)CtfFlags[i]->PropertiesPointer);
				}
			}

			if (gameTime > HalfTimeEnd)
			{
				htCtfEnd();
				HalfTimeState = HT_COMPLETED;
			}
			break;
		}
	}
	
}

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
void halftimeLogic(GameModule * module)
{
	int timeLimit = gameGetRawTimeLimit();
	int gameTime = getGameTime();
	GameSettings * gameSettings = getGameSettings();

	// Check we're in game and that it is compatible
	if (!gameSettings || gameSettings->GameRules != GAMERULE_CTF)
		return;

	// 
	switch (HalfTimeState)
	{
		case HT_WAITING:
		{
			if (timeLimit <= 0)
				break;

			// Trigger halfway through game
			u32 gameHalfTime = gameSettings->GameStartTime + (timeLimit / 2);
			if (gameTime > gameHalfTime)
			{
				htCtfBegin();
				HalfTimeState = HT_INTERMISSION;
			}
			break;
		}
		case HT_COMPLETED:
		{
			break;
		}
		default:
		{
			htCtfTick();
			break;
		}
	}
}

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
	HalfTimeState = 0;
	HalfTimeEnd = -1;
	CtfFlags[0] = CtfFlags[1] = CtfFlags[2] = CtfFlags[3] = 0;
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
		HalfTimeState = HT_INTERMISSION;
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
