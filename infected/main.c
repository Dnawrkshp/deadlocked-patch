/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Infected entrypoint and logic.
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
 * Infected team.
 */
#define INFECTED_TEAM			(TEAM_GREEN)

/*
 *
 */
int InfectedMask = 0;

/*
 *
 */
int WinningTeam = 0;

/*
 *
 */
int Initialized = 0;

/*
 * 
 */
char InfectedPopupBuffer[64];

/*
 *
 */
const char * InfectedPopupFormat = "%s has been infected!";

/*
 * NAME :		isInfected
 * 
 * DESCRIPTION :
 * 			Returns true if the given player is infected.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 		playerId:	Player index
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
inline int isInfected(int playerId)
{
	return InfectedMask & (1 << playerId);
}

/*
 * NAME :		infect
 * 
 * DESCRIPTION :
 * 			Infects the given player.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 		playerId:	Player index
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void infect(int playerId)
{
	InfectedMask |= (1 << playerId);

	GameSettings * gameSettings = GLOBAL_GAMESETTINGS;
	if (!gameSettings)
		return;

	InfectedPopupBuffer[0] = 0;
	sprintf(InfectedPopupBuffer, InfectedPopupFormat, gameSettings->PlayerNames[playerId]);
	InfectedPopupBuffer[63] = 0;

	showPopup(0, InfectedPopupBuffer);
	showPopup(1, InfectedPopupBuffer);
}

/*
 * NAME :		processPlayer
 * 
 * DESCRIPTION :
 * 			Process player.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 		player:		Player to process.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void processPlayer(Player * player)
{
	if (!player)
		return;

	int teamId = player->Team;
	short deaths = PLAYER_DEATHS_START[player->PlayerId];

	// No respawn
	player->RespawnTimer = 0x27;

	// 

	if (isInfected(player->PlayerId))
	{
		// If not on the right team then set it
		if (teamId != INFECTED_TEAM)
			changeTeam(player, INFECTED_TEAM);

		player->Speed = 4.0;
		player->DamageMultiplier = 1.001;
		player->QuadTimer = 0x1000;
		
		// Force wrench
		if (player->WeaponHeldId != WEAPON_ID_WRENCH &&
			player->WeaponHeldId != WEAPON_ID_SWINGSHOT)
			changeWeapon(player, WEAPON_ID_WRENCH);
	}
	// If the player is already on the infected team
	// or if they've died
	// then infect them
	else if (teamId == INFECTED_TEAM || deaths > 0)
	{
		infect(player->PlayerId);
	}
	// Process survivor logic
	else
	{

	}
}

/*
 * NAME :		getRandomSurvivor
 * 
 * DESCRIPTION :
 * 			Returns a random survivor.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 		seed :		Used to determine the random survivor.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
Player * getRandomSurvivor(u32 seed)
{
	Player ** playerObjects = PLAYER_STRUCT_ARRAY;

	int value = (seed % GAME_MAX_PLAYERS) + 1;
	int i = 0;
	int counter = 0;

	while (counter < value)
	{
		for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		{
			if (playerObjects[i] && !isInfected(playerObjects[i]->PlayerId))
			{
				++counter;

				if (value == counter)
					return playerObjects[i];
			}
		}

		// This means that there are no survivors left
		if (counter == 0)
			return NULL;
	}

	return NULL;
}

/*
 * NAME :		initialize
 * 
 * DESCRIPTION :
 * 			Initializes the gamemode.
 * 
 * NOTES :
 * 			This is called only once at the start.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void initialize(void)
{
	// No packs
	cheatsApplyNoPacks();

	// Disable health boxes
	if (cheatsDisableHealthboxes())
		Initialized = 1;
}

/*
 * NAME :		gameStart
 * 
 * DESCRIPTION :
 * 			Infected game logic entrypoint.
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
void gameStart(void)
{
	int i = 0;
	int infectedCount = 0;
	int playerCount = 0;
	GameSettings * gameSettings = GLOBAL_GAMESETTINGS;
	Player ** players = PLAYER_STRUCT_ARRAY;

	// Ensure in game
	if (!gameSettings || !isInGame())
		return;

	if (!Initialized)
		initialize();

	if (!GAME_HAS_ENDED)
	{
		// Reset to infected team
		// If one player isn't infected then their team
		// is set to winning team
		WinningTeam = INFECTED_TEAM;

		// Iterate through players
		for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		{
			if (!players[i])
				continue;
			
			// Process
			processPlayer(players[i]);

			// Count
			++playerCount;
			if (isInfected(players[i]->PlayerId))
			{
				++infectedCount;
			}
			else
			{
				WinningTeam = players[i]->Team;
			}
		}
	}

	// 
	setWinner(WinningTeam);

	if (!GAME_HAS_ENDED)
	{
		// If no survivors then end game
		if (playerCount == infectedCount && GAME_TIME_LIMIT > 0)
		{
			// End game
			endGame(WinningTeam, 0);
		}
		else if (infectedCount == 0)
		{
			// Infect first player after 10 seconds
			if ((GAME_TIME - gameSettings->GameStartTime) > (10 * TIME_SECOND))
			{
				Player * survivor = getRandomSurvivor(gameSettings->GameStartTime);
				if (survivor)
				{
					infect(survivor->PlayerId);
				}
			}
		}
	}

	return;
}

/*
 * NAME :		lobbyStart
 * 
 * DESCRIPTION :
 * 			Infected lobby logic entrypoint.
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
void lobbyStart(void)
{

}
