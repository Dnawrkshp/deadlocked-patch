/***************************************************
 * FILENAME :		gamesettings.h
 * 
 * DESCRIPTION :
 * 		Contains lobby specific offsets and structures for Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _GAMESETTINGS_H_
#define _GAMESETTINGS_H_

#include <tamtypes.h>

/*
 * Maximum number of players in a game.
 */
#define GAME_MAX_PLAYERS                (10)

/*
 *
 */
enum GameRuleIds
{
    GAMERULE_CQ,
    GAMERULE_CTF,
    GAMERULE_DM,
    GAMERULE_KOTH,
    GAMERULE_JUGGY
};

/*
 * NAME :		GameSettings
 * 
 * DESCRIPTION :
 * 			Contains the lobby's game settings.
 * 
 * NOTES :
 * 
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
typedef struct GameSettings
{
    // 
    char PlayerNames[GAME_MAX_PLAYERS][16];

    // 
    char PlayerClanTags[GAME_MAX_PLAYERS][8];

    //
    char PlayerSkins[GAME_MAX_PLAYERS];

    // 
    char PlayerTeams[GAME_MAX_PLAYERS];

    // 
    char PlayerClients[GAME_MAX_PLAYERS];

    // 
    char PlayerStates[GAME_MAX_PLAYERS];

    // 
    int UNK_118;

    // 
    char UNK_11C[8];

    //
    float PlayerRanks[GAME_MAX_PLAYERS];

    //
    float PlayerRankDeviations[GAME_MAX_PLAYERS];

    //
    int PlayerAccountIds[GAME_MAX_PLAYERS];

    //
    int GameStartTime;

    //
    int GameLoadStartTime;

    //
    short GameLevel;

    // 
    char PlayerCount;

    // 
    char SuperCheat;

    // 
    char PlayerCountAtStart;

    // 
    char GameRules;

    // 
    char GameType;

    // 
    short PlayerHeadset;

    // 
    char PlayerNamesOn;

    //
    char TeamSpawnPointIds[GAME_MAX_PLAYERS];

    // 
    u32 SpawnSeed;

} GameSettings;

/*
 * NAME :		getGameSettings
 * 
 * DESCRIPTION :
 * 			Returns a pointer to the active tNW_GameSettings object.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 *          Returns NULL if no lobby.
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
GameSettings * getGameSettings(void);

/*
 * NAME :		getGameRespawnTime
 * 
 * DESCRIPTION :
 * 			Gets the game's respawn timer setting.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
u8 getGameRespawnTime(void);

/*
 * NAME :		setGameRespawnTime
 * 
 * DESCRIPTION :
 * 			Sets the game's respawn timer setting.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setGameRespawnTime(u8 seconds);

/*
 * NAME :		getGameTimeLimit
 * 
 * DESCRIPTION :
 * 			Gets the game's time limit in minutes.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
u8 getGameTimeLimit(void);

/*
 * NAME :		setGameTimeLimit
 * 
 * DESCRIPTION :
 * 			Sets the game's time limit in minutes.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setGameTimeLimit(u8 minutes);

/*
 * NAME :		getGameSurvivor
 * 
 * DESCRIPTION :
 * 			Gets the game's survivor flag (no respawning)
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
u8 getGameSurvivor(void);

/*
 * NAME :		setGameSurvivor
 * 
 * DESCRIPTION :
 * 			Sets the game's survivor flag (no respawning)
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setGameSurvivor(u8 survivor);

/*
 * NAME :		getGameKillsToWin
 * 
 * DESCRIPTION :
 * 			Gets the game's kills to win target.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
u8 getGameKillsToWin(void);

/*
 * NAME :		setGameKillsToWin
 * 
 * DESCRIPTION :
 * 			Sets the game's kills to win target.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setGameKillsToWin(u8 kills);

/*
 * NAME :		flagSetPickupDistance
 * 
 * DESCRIPTION :
 * 			Sets the distance from a flag a player can pick up or save it.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void flagSetPickupDistance(float distance);

#endif // _GAMESETTINGS_H_
