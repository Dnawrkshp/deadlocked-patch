#include <tamtypes.h>
#include "gamesettings.h"


/*
 * Points to the active game settings.
 */
#define GLOBAL_GAMESETTINGS			((GameSettings*)(*(u32*)0x0021DFE8))

/*
 *
 */
#define GAME_RESPAWN_TIME           (*(u8*)0x002126DC)

/*
 *
 */
#define GAME_TIMELIMT_MINUTES       (*(u8*)0x002126D0)

/*
 *
 */
#define GAME_KILLS_TO_WIN           (*(u8*)0x002126D1)



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
GameSettings * getGameSettings(void)
{
    return GLOBAL_GAMESETTINGS;
}

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
u8 getGameRespawnTime(void)
{
    return GAME_RESPAWN_TIME;
}

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
void setGameRespawnTime(u8 seconds)
{
    GAME_RESPAWN_TIME = seconds;
}

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
u8 getGameTimeLimit(void)
{
    return GAME_TIMELIMT_MINUTES;
}

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
void setGameTimeLimit(u8 minutes)
{
    GAME_TIMELIMT_MINUTES = minutes;
}

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
u8 getGameKillsToWin(void)
{
    return GAME_KILLS_TO_WIN;
}

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
void setGameKillsToWin(u8 kills)
{
    GAME_KILLS_TO_WIN = kills;
}
