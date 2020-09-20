/***************************************************
 * FILENAME :		game.h
 * 
 * DESCRIPTION :
 * 		Contains game specific offsets and structures for Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _GAME_H_
#define _GAME_H_

#include <tamtypes.h>
#include "gamesettings.h"


//--------------------------------------------------------
typedef struct PlayerGameStats
{
    short RoadKills[GAME_MAX_PLAYERS];
    short RoadKillDeaths[GAME_MAX_PLAYERS];
    short UNK0[GAME_MAX_PLAYERS];
    short UNK1[GAME_MAX_PLAYERS];
    short Kills[GAME_MAX_PLAYERS];
    short Deaths[GAME_MAX_PLAYERS];
    short Suicides[GAME_MAX_PLAYERS];
    short UNK2[GAME_MAX_PLAYERS];
    short UNK3[GAME_MAX_PLAYERS];
    short WrenchKills[GAME_MAX_PLAYERS];
    short UNK4[GAME_MAX_PLAYERS];
    short UNK5[GAME_MAX_PLAYERS];
    char FlagsCapped[GAME_MAX_PLAYERS];
    char FlagsSaved[GAME_MAX_PLAYERS];
    float HillTime[GAME_MAX_PLAYERS];
} PlayerGameStats;

//--------------------------------------------------------
typedef struct PlayerWeaponStats
{
    u16 WeaponKills[GAME_MAX_PLAYERS][9];
    u16 WeaponDeaths[GAME_MAX_PLAYERS][9];
} PlayerWeaponStats;


//--------------------------------------------------------
typedef struct ScoreboardItem
{
    int TeamId;
    int UNK;
    int Value;
} ScoreboardItem;


/*
 * NAME :		showPopup
 * 
 * DESCRIPTION :
 * 			Shows a popup with the given text to the given local player.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      localPlayerIndex    :               Local player to show popup for.
 *      message             :               Message to show.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void showPopup(int localPlayerIndex, const char * message);

/*
 * NAME :		showHelpPopup
 * 
 * DESCRIPTION :
 * 			Shows a help popup with the given text to the given local player.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      localPlayerIndex    :               Local player to show popup for.
 *      message             :               Message to show.
 *      seconds             :               How many seconds to show the popup for.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void showHelpPopup(int localPlayerIndex, const char * message, int seconds);

/*
 * NAME :		setWinner
 * 
 * DESCRIPTION :
 * 			Set the winning team/player
 * 
 * NOTES :
 * 
 * ARGS : 
 *      teamOrPlayerId      :               Team/player id to set
 *      isTeam              :               Indicates if the given the winner is a team or player.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setWinner(int teamOrPlayerId, int isTeam);

/*
 * NAME :		endGame
 * 
 * DESCRIPTION :
 * 			End game.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      reason :      Why the game ended.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void endGame(int reason);

/*
 * NAME :		isInGame
 * 
 * DESCRIPTION :
 * 			Whether the client is currently in a game.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int isInGame(void);

/*
 * NAME :		hasGameEnded
 * 
 * DESCRIPTION :
 * 			Whether the game has ended and/or is ending.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int hasGameEnded(void);

/*
 * NAME :		getGameTime
 * 
 * DESCRIPTION :
 * 			Gets the current game time in milliseconds.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int getGameTime(void);

/*
 * NAME :		getGameFinishedExitTime
 * 
 * DESCRIPTION :
 * 			Gets the time when to leave after the game has ended.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 *          Returns 0 if the game has not ended.
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int getGameFinishedExitTime(void);

/*
 * NAME :		getDeathHeight
 * 
 * DESCRIPTION :
 * 			Gets the level's death height.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
float getDeathHeight(void);

/*
 * NAME :		setDeathHeight
 * 
 * DESCRIPTION :
 * 			Sets the level's death height.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setDeathHeight(float height);

/*
 * NAME :		getPlayerGameStats
 * 
 * DESCRIPTION :
 * 			Gets all the player stats relevant to the current game.
 * 
 * NOTES :
 *          This includes kills, deaths, suicides, hill time, flags capped, wrench kills, etc.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
PlayerGameStats * getPlayerGameStats(void);

/*
 * NAME :		getPlayerWeaponStats
 * 
 * DESCRIPTION :
 * 			Gets all the player weapon stats.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
PlayerWeaponStats * getPlayerWeaponStats(void);

#endif // _GAME_H_
