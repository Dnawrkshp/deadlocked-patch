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

#include "dl.h"
#include "player.h"
#include "pad.h"
#include "time.h"
#include "module.h"
#include "game.h"
#include "string.h"
#include "stdio.h"
#include "gamesettings.h"
#include "dialog.h"



/*
 * Array of game modules.
 */
#define GLOBAL_GAME_MODULES_START			((GameModule*)0x000CF000)

/*
 * Camera speed patch offsets.
 * Each offset is used by PatchCameraSpeed() to change the max
 * camera speed setting in game.
 */
#define CAMERA_SPEED_PATCH_OFF1			(*(u16*)0x00561BB8)
#define CAMERA_SPEED_PATCH_OFF2			(*(u16*)0x00561BDC)

#define ANNOUNCEMENTS_CHECK_PATCH		(*(u32*)0x00621D58)

#define GAMESETTINGS_LOAD_PATCH			(*(u32*)0x0072C3FC)
#define GAMESETTINGS_LOAD_FUNC			(0x0072EF78)
#define GAMESETTINGS_GET_INDEX_FUNC		(0x0070C410)
#define GAMESETTINGS_GET_VALUE_FUNC		(0x0070C538)


// 
void processSpectate(void);


/*
 * NAME :		patchCameraSpeed
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
void patchCameraSpeed()
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
 * NAME :		patchAnnouncements
 * 
 * DESCRIPTION :
 * 			Patches in-game announcements to work in all gamemodes.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void patchAnnouncements()
{
	if (ANNOUNCEMENTS_CHECK_PATCH == 0x907E01A9)
	{
		ANNOUNCEMENTS_CHECK_PATCH = 0x241E0000;
	}
}

/*
 * NAME :		patchGameSettingsLoad_Save
 * 
 * DESCRIPTION :
 * 			Saves the given value to the respective game setting.
 * 			I think this does validation depending on the input type then saves.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 			a0:					Points to the general settings area
 * 			offset0:			Offset to game setting
 * 			offset1:			Offset to game setting input type handler?
 * 			value:				Value to save
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void patchGameSettingsLoad_Save(void * a0, int offset0, int offset1, int value)
{
	u32 step1 = *(u32*)((u32)a0 + offset0);
	u32 step2 = *(u32*)(step1 + 0x58);
	u32 step3 = *(u32*)(step2 + offset1);
	((void (*)(u32, int))step3)(step1, value);
}

/*
 * NAME :		patchGameSettingsLoad_Hook
 * 
 * DESCRIPTION :
 * 			Called when loading previous game settings into create game.
 * 			Reloads Survivor correctly.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void patchGameSettingsLoad_Hook(void * a0, void * a1)
{
	int index = 0;
	int value = 0;

	// Load normal
	((void (*)(void *, void *))GAMESETTINGS_LOAD_FUNC)(a0, a1);

	// Get gametype
	index = ((int (*)(int))GAMESETTINGS_GET_INDEX_FUNC)(5);
	value = ((int (*)(void *, int))GAMESETTINGS_GET_VALUE_FUNC)(a1, index);

	// Handle each gamemode separately
	switch (value)
	{
		case GAMERULE_DM:
		{
			// Get survivor
			index = ((int (*)(int))GAMESETTINGS_GET_INDEX_FUNC)(9);
			value = ((int (*)(void *, int))GAMESETTINGS_GET_VALUE_FUNC)(a1, index);

			// Save survivor
			patchGameSettingsLoad_Save(a0, 0x100, 0xA4, value);
			break;
		}
	}
}

/*
 * NAME :		patchGameSettingsLoad
 * 
 * DESCRIPTION :
 * 			Patches game settings load so it reloads Survivor correctly.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void patchGameSettingsLoad()
{
	if (GAMESETTINGS_LOAD_PATCH == 0x0C1CBBDE)
	{
		GAMESETTINGS_LOAD_PATCH = 0x0C000000 | ((u32)&patchGameSettingsLoad_Hook >> 2);
	}
}

/*
 * NAME :		processGameModules
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
void processGameModules()
{
	// Start at the first game module
	GameModule * module = GLOBAL_GAME_MODULES_START;

	// Game settings
	GameSettings * gamesettings = getGameSettings();

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
				if (isInGame())
				{
					// Check if the game hasn't ended
					// We also give the module a second after the game has ended to
					// do some end game logic
					if (!hasGameEnded() || getGameTime() < (getGameFinishedExitTime() + TIME_SECOND))
					{
						// Invoke module
						if (module->GameEntrypoint)
							module->GameEntrypoint(module);
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
					if (gamesettings->GameStartTime > 0 && getGameTime() > gamesettings->GameStartTime && hasGameEnded() && module->State == GAMEMODULE_TEMP_ON)
						module->State = GAMEMODULE_OFF;
					// Invoke lobby module if still active
					else if (module->LobbyEntrypoint)
					{
						module->LobbyEntrypoint(module);
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
		else if (module->State == GAMEMODULE_ALWAYS_ON)
		{
			// Invoke lobby module if still active
			if (!isInGame() && module->LobbyEntrypoint)
			{
				module->LobbyEntrypoint(module);
			}
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
	// Call this first
	dlPreUpdate();

	// Patch camera speed
	patchCameraSpeed();

	// Patch announcements
	patchAnnouncements();

	// Patch game settings load
	patchGameSettingsLoad();

	// Process game modules
	processGameModules();

	// Process spectate
	processSpectate();

	// Call this last
	dlPostUpdate();

	return 0;
}
