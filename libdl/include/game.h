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
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void showHelpPopup(int localPlayerIndex, const char * message);

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


#endif // _GAME_H_
