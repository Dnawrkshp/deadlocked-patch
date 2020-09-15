/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Manages and applies all Deadlocked patches.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#include <tamtypes.h>

#include "appid.h"
#include "time.h"
#include "module.h"
#include "game.h"
#include "gamesettings.h"



#if APPID == DL_APPID

/*
 * ------------------------------------------------
 * ----------- START DEADLOCKED OFFSETS -----------
 * ------------------------------------------------
 */

/*
 * Camera speed patch offsets.
 * Each offset is used by PatchCameraSpeed() to change the max
 * camera speed setting in game.
 */
#define CAMERA_SPEED_PATCH_OFF1			(*(u16*)0x00561BB8)
#define CAMERA_SPEED_PATCH_OFF2			(*(u16*)0x00561BDC)


/*
 * ------------------------------------------------
 * ------------ END DEADLOCKED OFFSETS ------------
 * ------------------------------------------------
 */

#endif



/*
 * NAME :		PatchCameraSpeed
 * 
 * DESCRIPTION :
 * 			Patches in-game camera speed setting to max out at 200%.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void PatchCameraSpeed()
{
	// Check if the value is the default max of 64
	// This is to ensure that we only write here when
	// we're in game and the patch hasn't already been applied
	if (CAMERA_SPEED_PATCH_OFF1 == 0x40)
	{
		CAMERA_SPEED_PATCH_OFF1 = 0x80;
		CAMERA_SPEED_PATCH_OFF2 = 0x81;
	}
}


/*
 * NAME :		ProcessGameModules
 * 
 * DESCRIPTION :
 * 
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void ProcessGameModules()
{
	// Start at the first game module
	GameModule * module = GLOBAL_GAME_MODULES_START;

	// Game settings
	GameSettings * gamesettings = GLOBAL_GAMESETTINGS;

	// Iterate through all the game modules until we hit an empty one
	while (module->GameEntrypoint || module->LobbyEntrypoint)
	{
		// Ensure we have game settings
		if (gamesettings)
		{
			// Check the module is enabled
			if (module->State > GAMEMODULE_OFF)
			{
				// If in game, run game entrypoint
				if (GAME_ACTIVE)
				{
					// Check if the game hasn't ended
					// We also give the module a second after the game has ended to
					// do some end game logic
					if (!GAME_HAS_ENDED || GAME_TIME < (GAME_TIME_ENDGAME + TIME_SECOND))
					{
						// Invoke module
						if (module->GameEntrypoint)
							module->GameEntrypoint(module->Arg0);
					}
					// Game has ended so turn off if temporarily on
					else if (module->State == GAMEMODULE_TEMP_ON)
					{
						module->State = GAMEMODULE_OFF;
					}
				}
				else
				{
					// If the game has started and we're no longer in game
					// Then it must have ended
					if (gamesettings->GameStartTime > 0)
					{
						if (module->State == GAMEMODULE_TEMP_ON)
						{
							module->State = GAMEMODULE_OFF;
						}
					}
					// Invoke module if in staging
					else if (module->LobbyEntrypoint)
					{
						module->LobbyEntrypoint(module->Arg0);
					}
				}
			}
		}
		// If we aren't in a game then try to turn the module off
		// ONLY if it's temporarily enabled
		else if (module->State == GAMEMODULE_TEMP_ON)
		{
			module->State = GAMEMODULE_OFF;
		}

		++module;
	}
}


/*
 * NAME :		main
 * 
 * DESCRIPTION :
 * 			Applies all patches and modules.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int main (void)
{
	// Patch camera speed
	PatchCameraSpeed();

	// Process game modules
	ProcessGameModules();

	return 0;
}
