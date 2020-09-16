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
#include "appid.h"
#include "gamesettings.h"



#if APPID == DL_APPID

/*
 * ------------------------------------------------
 * ----------- START DEADLOCKED OFFSETS -----------
 * ------------------------------------------------
 */

/*
 * 
 */
#define GAME_ACTIVE                         (*(int*)0x0021E1EC)


/*
 * How many milliseconds for the game to last.
 * Negative if no time limit.
 */
#define GAME_TIME_LIMIT                     (*(int*)0x0036D600)

#define GAME_START_TIME                     (*(int*)0x0036D604)

/*
 * Game winner team id.
 */
#define GAME_WINNER_TEAM_ID                 (*(int*)0x0036D610)

/*
 * Player id of the winner. Set to -1 for team win.
 */
#define GAME_WINNER_PLAYER_ID               (*(int*)0x0036D614)

/*
 * Time (ms) that the game ended.
 */
#define GAME_TIME_ENDGAME                   (*(u32*)0x0036D664)

/*
 * Whether or not the game has ended.
 */
#define GAME_HAS_ENDED                      (GAME_TIME_ENDGAME > 0)

/*
 * Game time (ms).
 */
#define GAME_TIME                           (*(int*)0x00172378)

/*
 *
 */
#define PLAYER_KILLS_START                  ((short*)0x0036DA18)

/*
 *
 */
#define PLAYER_DEATHS_START                 ((short*)0x0036DA2C)

/*
 *
 */
#define PLAYER_SUICIDES_START               ((short*)0x0036DA40)

/*
 *
 */
#define PLAYER_WEAPON_STATS_ARRAY           ((PlayerWeaponStats*)0x0036D6A8)

/*
 * When non-zero, it refreshes the in-game scoreboard.
 */
#define GAME_SCOREBOARD_REFRESH_FLAG        (*(u32*)0x00310248)

/*
 *
 */
#define GAME_SCOREBOARD_TARGET              (*(u32*)0x002FA084)

/*
 * 
 */
#define GAME_SCOREBOARD_ARRAY               ((ScoreboardItem**)0x002FA04C)

/*
 * 
 */
#define GAME_SCOREBOARD_ITEM_COUNT          (*(u32*)0x002F9FCC)


/*
 * ------------------------------------------------
 * ------------ END DEADLOCKED OFFSETS ------------
 * ------------------------------------------------
 */

#endif

typedef struct PlayerWeaponStats
{
    u16 WeaponKills[GAME_MAX_PLAYERS][9];
    u16 WeaponDeaths[GAME_MAX_PLAYERS][9];
} PlayerWeaponStats;


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
extern void (* showPopup)(int localPlayerIndex, const char * message);


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
extern void setWinner(int teamOrPlayerId, int isTeam);

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
extern void endGame(int reason);
/*
 * NAME :		forceGameEnd
 * 
 * DESCRIPTION :
 * 			End the game and indicate if a player or team has won.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      isPlayerWinner  :        Indicates if the winner is a player or a team.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
extern void (* ForceGameEnd)(int isPlayerWinner);

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
extern int isInGame(void);

#endif // _GAME_H_
