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
 * How many milliseconds for the game to last.
 * Negative if no time limit.
 */
#define GAME_TIME_LIMIT                     (*(int*)0x0036D600)

#define GAME_START_TIME                     (*(int*)0x0036D604)

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
#define PLAYER_DEATHS_START                 ((short*)0x0036DA2C)

/*
 *
 */
#define PLAYER_SUICIDES_START               ((short*)0x0036DA40)

/*
 *
 */
#define GAME_WINNER                         (*(u32*)0x001E0D94)


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
 *
 */
extern void (* ShowPopup)(int, const char *);


/*
 *
 */
extern void SetWinner(int teamOrPlayerId);

/*
 *
 */
extern void EndGame(int winner, int isPlayer);

/*
 *
 */
extern void (* ForceGameEnd)(int);

/*
 * 
 */
extern void GameDisableRanked(void);

#endif // _GAME_H_
