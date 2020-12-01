/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Handles all gun game logic.
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
#include "player.h"
#include "weapon.h"
#include "hud.h"
#include "cheats.h"
#include "sha1.h"
#include "dialog.h"

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
char GunGameWeaponIds[] =
{
	WEAPON_SLOT_VIPERS,
	WEAPON_SLOT_B6,
	WEAPON_SLOT_MAGMA_CANNON,
	WEAPON_SLOT_ARBITER,
	WEAPON_SLOT_FLAIL,
	WEAPON_SLOT_FUSION_RIFLE,
	WEAPON_SLOT_MINE_LAUNCHER,
	WEAPON_SLOT_OMNI_SHIELD,
	WEAPON_SLOT_WRENCH,
	-1
};

/*
 * 
 */
const int GUN_INDEX_END = 9;

/*
 * 
 */
struct GunGameState
{
	int GunIndex;
	int LastGunKills;
	int LastWrenchDeaths;
	int LastSuicides;
#if DEBUG
	int PadReset;
#endif
} PlayerGunGameStates[GAME_MAX_PLAYERS];

/*
 * 
 */
struct WeaponModState
{
	int Omega;
	int Alpha[10];
} WeaponModStates[16];

/*
 *
 */
ScoreboardItem PlayerScores[GAME_MAX_PLAYERS];

/*
 *
 */
ScoreboardItem * SortedPlayerScores[GAME_MAX_PLAYERS];

/*
 *
 */
int WinningTeam = -1;

/*
 * 
 */
int GameOver = 0;

/*
 *
 */
int ScoreboardChanged = 0;

/*
 *
 */
int Initialized = 0;

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
 * 		player			:		Target player's player object.
 * 		playerState 	:		Target player's gun game state.
 * 		playerWepStats 	:		Target player's weapon stats.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void sortScoreboard(void)
{
	int i = 0;
	int j = 0;

	// bubble sort
	for (j = GAME_MAX_PLAYERS - 1; j > 0; --j)
	{
		for (i = 0; i < j; ++i)
		{
			// Swap
			if (SortedPlayerScores[i]->TeamId < 0 ||
				(SortedPlayerScores[i]->UNK != 1 &&
				 (SortedPlayerScores[i]->Value < SortedPlayerScores[i+1]->Value || 
				 SortedPlayerScores[i+1]->UNK == 1)))
			//	(SortedPlayerScores[i]->Value == SortedPlayerScores[i+1]->Value &&
			//	 SortedPlayerScores[i]->TeamId > SortedPlayerScores[i+1]->TeamId))
			{
				ScoreboardItem * temp = SortedPlayerScores[i];
				SortedPlayerScores[i] = SortedPlayerScores[i+1];
				SortedPlayerScores[i+1] = temp;
			}
		}
	}
}

/*
 * NAME :		setWeapon
 * 
 * DESCRIPTION :
 * 			Gives a player the given weapon and sets the appropriate alpha mods.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 		player			:		Target player's player object.
 * 		wepData 		:		Target player's weapon data.
 * 		weaponId 		:		Target weapon id to set.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setWeapon(Player * player, PlayerWeaponData * wepData, int weaponId)
{
	// Give
	if (wepData[weaponId].Level < 0)
		giveWeapon(player, weaponId, 0);

	// Set alpha mods
	memcpy(&wepData[weaponId].AlphaMods, &WeaponModStates[weaponId].Alpha, 10 * sizeof(int));
}

/*
 * NAME :		demotePlayer
 * 
 * DESCRIPTION :
 * 			Demotes a given player.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 		player			:		Target player's player object.
 * 		playerState 	:		Target player's gun game state.
 * 		playerWepStats 	:		Target player's weapon stats.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void demotePlayer(Player * player, struct GunGameState * playerState, PlayerWeaponStats * playerWepStats)
{
	if (!player || !playerState || !playerWepStats)
		return;

	int playerId = player->PlayerId;
	PlayerGameStats * stats = getPlayerGameStats();
	playerState->LastWrenchDeaths = playerWepStats->WeaponDeaths[playerId][WEAPON_SLOT_WRENCH];
	playerState->LastSuicides = stats->Suicides[playerId];

	if (playerState->GunIndex <= 0)
	{
		playerState->GunIndex = 0;
		return;
	}

	// Decrement gun index
	playerState->GunIndex -= 1;
	playerState->LastGunKills = playerWepStats->WeaponKills[playerId][(int)GunGameWeaponIds[playerState->GunIndex]];

	// Show popup
	if (isLocal(player))
		showPopup(player->LocalPlayerIndex, "You've been demoted!");
}

/*
 * NAME :		promotePlayer
 * 
 * DESCRIPTION :
 * 			Promotes a given player.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 		player			:		Target player's player object.
 * 		playerState 	:		Target player's gun game state.
 * 		playerWepStats 	:		Target player's weapon stats.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void promotePlayer(Player * player, struct GunGameState * playerState, PlayerWeaponStats * playerWepStats)
{
	if (!player || !playerState || !playerWepStats)
		return;

	int playerId = player->PlayerId;
	PlayerGameStats * stats = getPlayerGameStats();

	// Player has already promoted as far as possible
	if (playerState->GunIndex >= GUN_INDEX_END)
	{
		playerState->GunIndex = GUN_INDEX_END;
		return;
	}

	// Increment gun index
	playerState->GunIndex += 1;
	playerState->LastGunKills = playerWepStats->WeaponKills[playerId][(int)GunGameWeaponIds[playerState->GunIndex]];
	playerState->LastWrenchDeaths = playerWepStats->WeaponDeaths[playerId][WEAPON_SLOT_WRENCH];
	playerState->LastSuicides = stats->Suicides[playerId];

	// Show popup
	if (isLocal(player))
	{
		showPopup(player->LocalPlayerIndex, "You've promoted to the next weapon!");

		// Generate our rng seed from SHA1 of spawn seed
		sha1(&shaBuffer, 4, (void*)&shaBuffer, 4);

		switch (shaBuffer & 3)
		{
			case 0:
			{
				playDialogSound(DIALOG_ID_TRY_THIS_ONE_ON_FOR_SIZE, 0);
				break;
			}
			case 1:
			{
				playDialogSound(DIALOG_ID_LETS_SEE_HOW_HE_DOES_WITH_THIS, 0);
				break;
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
void processPlayer(Player * player)
{
	if (!player)
		return;

	int playerId = player->PlayerId;
	struct GunGameState * playerState = &PlayerGunGameStates[player->PlayerId];
	PlayerGameStats * stats = getPlayerGameStats();
	char activeGunSlotId = GunGameWeaponIds[playerState->GunIndex];
	char activeGunId = weaponSlotToId(activeGunSlotId);
	PlayerWeaponStats * playerWepStats = getPlayerWeaponStats();
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
	playerWeaponData = getPlayerWeaponData(playerId);
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
		if (isLocal(player))
		{
			setLocalPlayerEquipslot(player->LocalPlayerIndex, 0, activeGunId);
			setLocalPlayerEquipslot(player->LocalPlayerIndex, 1, WEAPON_ID_EMPTY);
			setLocalPlayerEquipslot(player->LocalPlayerIndex, 2, WEAPON_ID_EMPTY);
		}

		changeWeapon(player, activeGunId);
	}
	
	// Check for demotion
	if (playerWepStats->WeaponDeaths[playerId][WEAPON_SLOT_WRENCH] > playerState->LastWrenchDeaths ||
		stats->Suicides[playerId] > playerState->LastSuicides)
		demotePlayer(player, playerState, playerWepStats);

	// Check for promotion
	else if (playerWepStats->WeaponKills[playerId][(int)activeGunSlotId] > playerState->LastGunKills)
		promotePlayer(player, playerState, playerWepStats);

#if DEBUG
	if (playerState->PadReset == 1 && (player->Paddata->btns & PAD_L3))
	{
		playerState->PadReset = 0;
	}
	else if (playerState->PadReset == 2 && (player->Paddata->btns & PAD_R3))
	{
		playerState->PadReset = 0;
	}
	else if (playerState->PadReset == 0 && !(player->Paddata->btns & PAD_L3) && playerState->GunIndex > 0)
	{
		playerState->PadReset = 1;
		DemotePlayer(player, playerState, playerWepStats);
	}
	else if (playerState->PadReset == 0 && !(player->Paddata->btns & PAD_R3) && playerState->GunIndex < (GUN_INDEX_END - 1))
	{
		playerState->PadReset = 2;
		PromotePlayer(player, playerState, playerWepStats);
	}
#endif

	// Update scoreboard
	if (PlayerScores[playerId].Value != playerState->GunIndex)
	{
		PlayerScores[playerId].Value = playerState->GunIndex;
		ScoreboardChanged = 1;
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
	u8 rngSeed[12];
	u8 rngBuf[12];
	GameSettings * gameSettings = getGameSettings();
	Player ** players = getPlayers();

	// Reset states to 0
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		PlayerGunGameStates[i].GunIndex = 0;
		PlayerGunGameStates[i].LastGunKills = 0;
		PlayerGunGameStates[i].LastWrenchDeaths = 0;
		PlayerGunGameStates[i].LastSuicides = 0;
		
#if DEBUG
		PlayerGunGameStates[i].PadReset = 0;
#endif
	}

	// Randomize middle tier weapons
	u32 seed = gameSettings->SpawnSeed;
	j = 1;
	while (seed)
	{
		// If lowest bit is set then swap with next id
		if (seed & 1)
		{
			char temp = GunGameWeaponIds[j];
			GunGameWeaponIds[j] = GunGameWeaponIds[j+1];
			GunGameWeaponIds[j+1] = temp;
		}

		// Increment weapon id index
		++j;
		
		// Ensure id is within the randomize bounds
		if (j > 5)
			j = 1;

		// Shift seed down
		seed >>= 1;
	}

	// Generate our rng seed from SHA1 of spawn seed
	sha1(&gameSettings->SpawnSeed, 4, (void*)rngSeed, 12);

	// Initialize weapon mods
	for (i = 0; i < 16; ++i)
	{
		// SHA1 of spawn seed
		// This is our way of generating random numbers
		sha1((const void*)rngSeed, 12, (void*)rngBuf, 12);
		memcpy(rngSeed, rngBuf, 12);

		WeaponModStates[i].Omega = 0;
		for (j = 0; j < 10; ++j)
			WeaponModStates[i].Alpha[j] = rngBuf[j] % 9;
	}

	// Initialize scoreboard
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		PlayerScores[i].TeamId = p ? i : 0;
		PlayerScores[i].UNK = isLocal(p);
		PlayerScores[i].Value = 0;
		GAME_SCOREBOARD_ARRAY[i] = p ? &PlayerScores[i] : 0;
		
		SortedPlayerScores[i] = &PlayerScores[i];
	}

	// No packs
	cheatsApplyNoPacks();

	// Set respawn time to 2
	setGameRespawnTime(2);

	// Set kill target to 0 (disable)
	setGameKillsToWin(0);

	// Set holoshields to instant kill
	setWeaponDamage(WEAPON_ID_OMNI_SHIELD, 0, 50);
	
	// Reset winning team to default
	WinningTeam = 0;

	// Reset gameover
	GameOver = 0;

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
	GameSettings * gameSettings = getGameSettings();
	Player ** players = getPlayers();

	// Ensure in game
	if (!gameSettings)
		return;

	if (!Initialized)
		initialize();

	if (!hasGameEnded() && !GameOver)
	{
		// Disable weapon hud
		getPlayerHUDFlags(0)->Weapons = 0;
		getPlayerHUDFlags(1)->Weapons = 0;

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
		sortScoreboard();
		GAME_SCOREBOARD_REFRESH_FLAG = 1;
		ScoreboardChanged = 0;
	}

	// Target to last gun index
	GAME_SCOREBOARD_TARGET = GUN_INDEX_END;

	// Set winner
	setWinner(WinningTeam, 0);

	// End game
	if (GameOver && !hasGameEnded())
	{
		endGame(2);
		ScoreboardChanged = 1;
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
