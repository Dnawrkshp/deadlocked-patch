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
#include "scoreboard.h"


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
void htReset(void)
{
    HalfTimeState = 0;
	HalfTimeEnd = -1;
	CtfFlags[0] = CtfFlags[1] = CtfFlags[2] = CtfFlags[3] = 0;
}

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
