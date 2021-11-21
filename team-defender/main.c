/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Handles all team defender logic.
 * 
 * NOTES :
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#include <tamtypes.h>
#include <string.h>

#include <libdl/time.h>
#include <libdl/game.h>
#include <libdl/gamesettings.h>
#include <libdl/player.h>
#include <libdl/weapon.h>
#include <libdl/hud.h>
#include <libdl/cheats.h>
#include <libdl/sha1.h>
#include <libdl/dialog.h>
#include <libdl/ui.h>
#include <libdl/stdio.h>
#include "module.h"

// TODO
// Create scoreboard abstraction in libdl

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
 * 
 */
typedef struct PlayerState
{
	Player * Player;
	int HasFlag;
	int Points;
} PlayerState_t;

/*
 * 
 */
struct GameState
{
	int WinningTeam;
	int GameOver;
  PlayerState_t Players[GAME_MAX_PLAYERS];
} GameState;

/*
 *
 */
ScoreboardItem TeamScores[GAME_MAX_PLAYERS];

/*
 *
 */
ScoreboardItem * SortedTeamScores[GAME_MAX_PLAYERS];

/*
 *
 */
int Initialized;

/*
 *
 */
int ScoreboardChanged = 0;

/*
 *
 */
char shaBuffer;


/*
 * NAME :		sortScoreboard
 * 
 * DESCRIPTION :
 * 			Sorts the scoreboard by value.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void sortScoreboard(int dontLockLocal)
{
	int i = 0;
	int j = 0;

	// bubble sort
	for (j = GAME_MAX_PLAYERS - 1; j > 0; --j)
	{
		for (i = 0; i < j; ++i)
		{
			// Swap
			if (SortedTeamScores[i]->TeamId < 0 ||
				((dontLockLocal || SortedTeamScores[i]->UNK != 1) &&
				 (SortedTeamScores[i]->Value < SortedTeamScores[i+1]->Value || 
				 (SortedTeamScores[i+1]->UNK == 1 && !dontLockLocal))))
			{
				ScoreboardItem * temp = SortedTeamScores[i];
				SortedTeamScores[i] = SortedTeamScores[i+1];
				SortedTeamScores[i+1] = temp;
			}
		}
	}
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
void processPlayer(PlayerState_t * playerState)
{
	if (!playerState || !playerState->Player)
		return;

	Player * player = playerState->Player;
	int playerId = player->PlayerId;
	struct GunGameState * playerState = &PlayerGunGameStates[player->PlayerId];
	PlayerGameStats * stats = gameGetPlayerStats();
	char activeGunSlotId = GunGameWeaponIds[playerState->GunIndex];
	char activeGunId = weaponSlotToId(activeGunSlotId);
	PlayerWeaponStats * playerWepStats = gameGetPlayerWeaponStats();
	PlayerWeaponData * playerWeaponData = NULL;

	// If player has reached end then game over
	if (playerState->GunIndex >= GUN_INDEX_END)
	{
		WinningTeam = player->PlayerId;
		GameOver = 1;
		return;
	}

	// Clamp bounds
	if (playerState->GunIndex < 0)
		playerState->GunIndex = 0;

	// Enable all weapons
	playerWeaponData = playerGetWeaponData(playerId);
	setWeapon(player, playerWeaponData, WEAPON_ID_VIPERS);
	setWeapon(player, playerWeaponData, WEAPON_ID_MAGMA_CANNON);
	setWeapon(player, playerWeaponData, WEAPON_ID_ARBITER);
	setWeapon(player, playerWeaponData, WEAPON_ID_FUSION_RIFLE);
	setWeapon(player, playerWeaponData, WEAPON_ID_MINE_LAUNCHER);
	setWeapon(player, playerWeaponData, WEAPON_ID_B6);
	setWeapon(player, playerWeaponData, WEAPON_ID_OMNI_SHIELD);
	setWeapon(player, playerWeaponData, WEAPON_ID_FLAIL);

	// Only allow swingshot, wrench and active gun
	if (player->WeaponHeldId != WEAPON_ID_WRENCH &&
		player->WeaponHeldId != WEAPON_ID_SWINGSHOT &&
		player->WeaponHeldId != activeGunId)
	{
		if (playerIsLocal(player))
		{
			playerSetLocalEquipslot(player->LocalPlayerIndex, 0, activeGunId);
			playerSetLocalEquipslot(player->LocalPlayerIndex, 1, WEAPON_ID_EMPTY);
			playerSetLocalEquipslot(player->LocalPlayerIndex, 2, WEAPON_ID_EMPTY);
		}

		playerSetWeapon(player, activeGunId);
	}
}

/*
 * NAME :		initialize
 * 
 * DESCRIPTION :
 * 			Initializes the game mode.
 * 			Resets states, generates random weapon ordering, generates random alpha mods.
 * 
 * NOTES :
 * 			This is called once at start.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void initialize(void)
{
	int i = 0;
	int j = 0;
	u32 seed;
	u8 rngBuf[12];
	GameSettings * gameSettings = gameGetSettings();
	GameOptions * gameOptions = gameGetOptions();
	Player ** players = playerGetAll();
	Player * localPlayer = NULL;
	int teamIds[GAME_MAX_PLAYERS];
	int teamsCount = 0;

	// Reset states to 0
	GameState.WinningTeam = -1;
	GameState.GameOver = 0;
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		if (playerIsLocal(players[i]))
			localPlayer = players[i];
		GameState.Players[i].Player = players[i];
		GameState.Players[i].HasFlag = 0;
		GameState.Players[i].Points = 0;

		// build team list
		if (players[i])
		{
			int pTeam = players[i]->Team;
			for (j = 0; j < teamsCount; ++j)
				if (teamIds[j] == pTeam)
					break;

			if (j > teamsCount)
				teamIds[teamsCount++] = pTeam;
		}
	}

	// Randomly pick first flag carrier
	sha1(&gameSettings->SpawnSeed, 4, &seed, 4);
	seed %= GAME_MAX_PLAYERS;
	i = 0;
	j = 0;
	PlayerState_t * pickedPlayer = NULL;
	while (i <= seed)
	{
		if ((pickedPlayer = &GameState.Players[j])->Player)
			++i;

		++j;
	}

	// 
	if (pickedPlayer && pickedPlayer->Player)
	{
		pickedPlayer->HasFlag = 1;
	}

	// Initialize scoreboard
	for (i = 0; i < teamsCount; ++i)
	{
		int teamExists = 0;
		int teamIsLocal = i == localPlayer->Team;
		for (j = 0; j < teamsCount; ++j)
		{
			if (teamIds[j] == i)
			{
				teamExists = 1;
				break;
			}
		}

		TeamScores[i].TeamId = teamExists ? i : -1;
		TeamScores[i].UNK = teamIsLocal;
		TeamScores[i].Value = 0;
		GAME_SCOREBOARD_ARRAY[i] = teamExists ? &TeamScores[i] : 0;
		
		SortedTeamScores[i] = &TeamScores[i];
	}

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
	GameSettings * gameSettings = gameGetSettings();
	Player ** players = playerGetAll();

	// Ensure in game
	if (!gameSettings)
		return;

	if (!Initialized)
		initialize();

	if (!gameHasEnded() && !GameOver)
	{
		// Iterate through players
		for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		{
			if (GameOver)
				break;

			// Force scoreboard to custom scoreboard values
			if ((players[i] || GAME_SCOREBOARD_ARRAY[i]) && GAME_SCOREBOARD_ARRAY[i] != SortedPlayerScores[i])
			{
				GAME_SCOREBOARD_ARRAY[i] = SortedPlayerScores[i];
				GAME_SCOREBOARD_REFRESH_FLAG = 1;
			}

			if (!players[i])
				continue;
			
			// 
			PlayerScores[i].TeamId = players[i]->PlayerId;

			// Process
			processPlayer(players[i]);
		}
	}
	else
	{
		// Correct scoreboard
		for (i = 0; i < GAME_SCOREBOARD_ITEM_COUNT; ++i)
		{
			// Force scoreboard to custom scoreboard values
			if (GAME_SCOREBOARD_ARRAY[i] != SortedPlayerScores[i])
			{
				GAME_SCOREBOARD_ARRAY[i] = SortedPlayerScores[i];
				GAME_SCOREBOARD_REFRESH_FLAG = 1;
			}
		}
	}

	// Update scoreboard on change
	if (ScoreboardChanged)
	{
		sortScoreboard(0);
		GAME_SCOREBOARD_REFRESH_FLAG = 1;
		ScoreboardChanged = 0;
	}

	// Target to last gun index
	GAME_SCOREBOARD_TARGET = GUN_INDEX_END;

	// Set winner
	gameSetWinner(WinningTeam, 0);

	// End game
	if (GameOver && !gameHasEnded())
	{
		gameEnd(2);
		ScoreboardChanged = 1;
	}

	return;
}

void setEndGameScoreboard(void)
{
	u32 * uiElements = (u32*)(*(u32*)(0x011C7064 + 4*18) + 0xB0);
	int i;
	char buf[24];

	// reset buf
	memset(buf, 0, sizeof(buf));

	// sort scoreboard again
	sortScoreboard(1);

	// names start at 6
	// column headers start at 17
	strncpy((char*)(uiElements[18] + 0x60), "POINTS", 4);
	strncpy((char*)(uiElements[19] + 0x60), "KILLS", 6);
	strncpy((char*)(uiElements[20] + 0x60), "DEATHS", 7);

	// rows
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		int pid = SortedTeamScores[i]->TeamId;
		if (pid >= 0)
		{
			// set points
			sprintf(buf, "%d", SortedTeamScores[i]->Value);
			strncpy((char*)(uiElements[22 + (i*4) + 0] + 0x60), buf, strlen(buf) + 1);

			// moves deaths over
			strncpy((char*)(uiElements[22 + (i*4) + 2] + 0x60), (char*)(uiElements[22 + (i*4) + 1] + 0x60), 8);
			
			// moves kills over
			strncpy((char*)(uiElements[22 + (i*4) + 1] + 0x60), (char*)(uiElements[22 + (i*4) + 0] + 0x60), 8);
		}
	}
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
	int activeId = uiGetActive();
	static int initializedScoreboard = 0;

	// scoreboard
	switch (activeId)
	{
		case 0x15C:
		{
			if (initializedScoreboard)
				break;

			setEndGameScoreboard();
			initializedScoreboard = 1;
			break;
		}
		case UI_ID_GAME_LOBBY:
		{
			setLobbyGameOptions();
			break;
		}
	}
}

/*
 * NAME :		loadStart
 * 
 * DESCRIPTION :
 * 			Load logic entrypoint.
 * 
 * NOTES :
 * 			This is called only when the game has finished reading the level from the disc and before it has started processing the data.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void loadStart(void)
{
	
}
