#include "game.h"


//--------------------------------------------------------
#define GAME_END_FUNC                       (0x006228C8)
#define GAME_SHOWPOPUP_FUNC                 (0x00540170)
#define GAME_SHOWHELP_FUNC                  (0x00540140)

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
#define GAME_DEATH_BARRIER_Y                (*(float*)0x0022267C)

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
void showPopup(int localPlayerIndex, const char * message)
{
    ((void(*)(int, const char *))GAME_SHOWPOPUP_FUNC)(localPlayerIndex, message);
}

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
void showHelpPopup(int localPlayerIndex, const char * message)
{
    ((void(*)(int, const char *))GAME_SHOWHELP_FUNC)(localPlayerIndex, message);
}

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
void setWinner(int teamOrPlayerId, int isTeam)
{
    GAME_WINNER_TEAM_ID = teamOrPlayerId;
    GAME_WINNER_PLAYER_ID = isTeam ? -1 : teamOrPlayerId;
}

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
void endGame(int reason)
{
    if (!hasGameEnded())
        ((void(*)(int))GAME_END_FUNC)(reason);
}

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
int isInGame(void)
{
    return GAME_ACTIVE;
}

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
int hasGameEnded(void)
{
    return GAME_HAS_ENDED;
}
