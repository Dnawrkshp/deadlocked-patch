/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Manages and applies all Deadlocked patches.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#include <tamtypes.h>

#include "stdio.h"
#include "time.h"
#include "module.h"
#include "game.h"
#include "gamesettings.h"
#include "player.h"
#include "infected.h"



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
 * NAME :		IsInfected
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
inline int IsInfected(int playerId)
{
	return InfectedMask & (1 << playerId);
}

void Infect(int playerId)
{
	InfectedMask |= (1 << playerId);

	GameSettings * gameSettings = GLOBAL_GAMESETTINGS;
	if (!gameSettings)
		return;

	InfectedPopupBuffer[0] = 0;
	sprintf(InfectedPopupBuffer, InfectedPopupFormat, gameSettings->PlayerNames[playerId]);
	InfectedPopupBuffer[63] = 0;

	ShowPopup(0, InfectedPopupBuffer);
	ShowPopup(1, InfectedPopupBuffer);
}

/*
 * NAME :		ProcessPlayer
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
void ProcessPlayer(Player * player)
{
	if (!player)
		return;

	int isInfected = IsInfected(player->PlayerId);
	int teamId = player->Team;
	short deaths = PLAYER_DEATHS_START[player->PlayerId];

	// No respawn
	player->RespawnTimer = 0x27;

	// 

	if (isInfected)
	{
		// If not on the right team then set it
		if (teamId != INFECTED_TEAM)
			ChangeTeam(player, INFECTED_TEAM);

		player->Speed = 4.0;
		player->DamageMultiplier = 1.001;
		player->QuadTimer = 0x1000;
		
		// Force wrench
		if (player->WeaponHeldId != WEAPON_ID_WRENCH &&
			player->WeaponHeldId != WEAPON_ID_SWINGSHOT)
			ChangeWeapon(player, WEAPON_ID_WRENCH);
	}
	// If the player is already on the infected team
	// or if they've died
	// then infect them
	else if (teamId == INFECTED_TEAM || deaths > 0)
	{
		Infect(player->PlayerId);
	}
	// Process survivor logic
	else
	{

	}
}

/*
 * NAME :		GetRandomSurvivor
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
Player * GetRandomSurvivor(u32 seed)
{
	Player ** playerObjects = PLAYER_STRUCT_ARRAY;

	int value = (seed % GAME_MAX_PLAYERS) + 1;
	int i = 0;
	int counter = 0;

	while (counter < value)
	{
		for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		{
			if (playerObjects[i] && !IsInfected(playerObjects[i]->PlayerId))
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
 * NAME :		main
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
void Initialize(void)
{
	// No packs
	*(u32*)0x00414660 = 0x03E00008;
	*(u32*)0x00414664 = 0x00000000;

	// Iterate through mobys and disable healthboxes
	Moby ** mobiesArray = MOBY_ARRAY;
	Moby * currentMoby;
	while ((currentMoby = *mobiesArray))
	{
		if (currentMoby->MobyId == MOBY_ID_HEALTH_BOX_MULT)
		{
			currentMoby->PositionX = 0;
			currentMoby->PositionY = 0;
			currentMoby->PositionZ = 0;

			if (currentMoby->PropertiesPointer)
			{
				void * subPtr = (void*)(*(u32*)(currentMoby->PropertiesPointer));
				if (subPtr)
				{
					Moby * orb = (Moby*)(*(u32*)(subPtr + 0x98));
					if (orb)
					{
						orb->PositionX = 0;
						orb->PositionY = 0;
						orb->PositionZ = 0;

						
						Initialized = 1;
					}
				}
				// 
			}
		}

		++mobiesArray;
	}

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
	if (!gameSettings)
		return;

	if (!Initialized)
		Initialize();

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
			ProcessPlayer(players[i]);

			// Count
			++playerCount;
			if (IsInfected(players[i]->PlayerId))
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
	SetWinner(WinningTeam);

	if (!GAME_HAS_ENDED)
	{
		// If no survivors then end game
		if (playerCount == infectedCount)
		{
			// End game
			EndGame(WinningTeam, 0);
		}
		else if (infectedCount == 0)
		{
			// Infect first player after 10 seconds
			if ((GAME_TIME - gameSettings->GameStartTime) > (10 * TIME_SECOND))
			{
				Player * survivor = GetRandomSurvivor(gameSettings->GameStartTime);
				if (survivor)
					Infect(survivor->PlayerId);
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
