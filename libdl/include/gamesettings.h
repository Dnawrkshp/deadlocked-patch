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

/*
 * Maximum number of players in a game.
 */
#define GAME_MAX_PLAYERS                (10)

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



#endif // _GAMESETTINGS_H_
