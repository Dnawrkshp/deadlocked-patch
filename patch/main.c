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

#include <libdl/dl.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/time.h>
#include <libdl/net.h>
#include "module.h"
#include "messageid.h"
#include "include/config.h"
#include <libdl/game.h>
#include <libdl/string.h>
#include <libdl/stdio.h>
#include <libdl/gamesettings.h>
#include <libdl/dialog.h>
#include <libdl/patch.h>
#include <libdl/ui.h>
#include <libdl/graphics.h>
#include <libdl/sha1.h>

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

#define UI_POINTERS						((u32*)0x011C7064)

#define ANNOUNCEMENTS_CHECK_PATCH		(*(u32*)0x00621D58)

#define GAMESETTINGS_LOAD_PATCH			(*(u32*)0x0072C3FC)
#define GAMESETTINGS_LOAD_FUNC			(0x0072EF78)
#define GAMESETTINGS_GET_INDEX_FUNC		(0x0070C410)
#define GAMESETTINGS_GET_VALUE_FUNC		(0x0070C538)

#define GAMESETTINGS_BUILD_PTR			(*(u32*)0x004B882C)
#define GAMESETTINGS_BUILD_FUNC			(0x00712BF0)

#define GAMESETTINGS_CREATE_PATCH		(*(u32*)0x0072E5B4)
#define GAMESETTINGS_CREATE_FUNC		(0x0070B540)

#define GAMESETTINGS_RESPAWN_TIME      	(*(char*)0x0017380C)
#define GAMESETTINGS_RESPAWN_TIME2      (*(char*)0x012B3638)

#define GAMESETTINGS_SURVIVOR			(*(u8*)0x00173806)

#define FRAME_SKIP_WRITE0				(*(u32*)0x004A9400)
#define FRAME_SKIP						(*(u32*)0x0021E1D8)

#define IS_PROGRESSIVE_SCAN					(*(int*)0x0021DE6C)

#define EXCEPTION_DISPLAY_ADDR			(0x000E0000)

#define SHRUB_RENDER_DISTANCE				(*(float*)0x0022308C)

#define DRAW_SHADOW_FUNC						((u32*)0x00587b30)

// 
void processSpectate(void);
void runMapLoader(void);
void onMapLoaderOnlineMenu(void);
void onConfigOnlineMenu(void);
void onConfigGameMenu(void);
void configMenuEnable(void);

// 
int hasInitialized = 0;
int sentGameStart = 0;
int lastMenuInvokedTime = 0;
int lastGameState = 0;
int hasInstalledExceptionHandler = 0;
const char * patchConfigStr = "PATCH CONFIG";

extern float _lodScale;
extern void* _correctTieLod;

typedef struct ChangeTeamRequest {
	u32 Seed;
	int PoolSize;
	char Pool[GAME_MAX_PLAYERS];
} ChangeTeamRequest_t;

//
const PlayerStateCondition_t stateSkipRemoteConditions[] = {
	{	// skip when player is swinging
		PLAYERSTATECONDITION_LOCAL_OR_REMOTE_EQUALS,
		0,
		43
	},
};

const PlayerStateCondition_t stateForceRemoteConditions[] = {
	{ // force chargebooting
		PLAYERSTATECONDITION_LOCAL_OR_REMOTE_EQUALS,
		0,
		125
	},
	{ // force remote if local is still wrenching
		PLAYERSTATECONDITION_LOCAL_EQUALS,
		15,
		19
	},
	{ // force remote if local is still hyper striking
		PLAYERSTATECONDITION_LOCAL_EQUALS,
		15,
		20
	}
};

// 
PatchConfig_t config __attribute__((section(".config"))) = {
	0,
	0,
	0,
	0,
	1,
	0
};

/*
 * NAME :		patchCameraSpeed
 * 
 * DESCRIPTION :
 * 			Patches in-game camera speed setting to max out at 300%.
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
	const u16 SPEED = 0x100;
	char buffer[16];

	// Check if the value is the default max of 64
	// This is to ensure that we only write here when
	// we're in game and the patch hasn't already been applied
	if (CAMERA_SPEED_PATCH_OFF1 == 0x40)
	{
		CAMERA_SPEED_PATCH_OFF1 = SPEED;
		CAMERA_SPEED_PATCH_OFF2 = SPEED+1;
	}

	// Patch edit profile bar
	if (uiGetActive() == UI_ID_EDIT_PROFILE)
	{
		void * editProfile = (void*)UI_POINTERS[30];
		if (editProfile)
		{
			// get cam speed element
			void * camSpeedElement = (void*)*(u32*)(editProfile + 0xC0);
			if (camSpeedElement)
			{
				// update max value
				*(u32*)(camSpeedElement + 0x78) = SPEED;

				// get current value
				float value = *(u32*)(camSpeedElement + 0x70) / 64.0;

				// render
				sprintf(buffer, "%.0f%%", value*100);
				gfxScreenSpaceText(240,   166,   1, 1, 0x80000000, buffer, -1, 1);
				gfxScreenSpaceText(240-1, 166-1, 1, 1, 0x80FFFFFF, buffer, -1, 1);
			}
		}
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
	u32 addrValue = ANNOUNCEMENTS_CHECK_PATCH;
	if (config.enableGamemodeAnnouncements && addrValue == 0x907E01A9)
		ANNOUNCEMENTS_CHECK_PATCH = 0x241E0000;
	else if (!config.enableGamemodeAnnouncements && addrValue == 0x241E0000)
		ANNOUNCEMENTS_CHECK_PATCH = 0x907E01A9;
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
	int gamemode = ((int (*)(void *, int))GAMESETTINGS_GET_VALUE_FUNC)(a1, index);

	// Handle each gamemode separately
	switch (gamemode)
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

	if (gamemode != GAMERULE_CQ)
	{
		// respawn timer
		GAMESETTINGS_RESPAWN_TIME2 = *(u8*)0x002126DC;
	}

	if (GAMESETTINGS_RESPAWN_TIME2 < 0)
		GAMESETTINGS_RESPAWN_TIME2 = 5;
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
 * NAME :		patchPopulateCreateGame_Hook
 * 
 * DESCRIPTION :
 * 			Patches create game populate setting to add respawn timer.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void patchPopulateCreateGame_Hook(void * a0, int settingsCount, u32 * settingsPtrs)
{
	u32 respawnTimerPtr = 0x012B35D8;
	int i = 0;

	// Check if already loaded
	for (; i < settingsCount; ++i)
	{
		if (settingsPtrs[i] == respawnTimerPtr)
			break;
	}

	// If not loaded then append respawn timer
	if (i == settingsCount)
	{
		++settingsCount;
		settingsPtrs[i] = respawnTimerPtr;
	}

	// Populate
	((void (*)(void *, int, u32 *))GAMESETTINGS_BUILD_FUNC)(a0, settingsCount, settingsPtrs);
}

/*
 * NAME :		patchPopulateCreateGame
 * 
 * DESCRIPTION :
 * 			Patches create game populate setting to add respawn timer.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void patchPopulateCreateGame()
{
	// Patch function pointer
	if (GAMESETTINGS_BUILD_PTR == GAMESETTINGS_BUILD_FUNC)
	{
		GAMESETTINGS_BUILD_PTR = (u32)&patchPopulateCreateGame_Hook;
	}

	// Patch default respawn timer
	GAMESETTINGS_RESPAWN_TIME = GAMESETTINGS_RESPAWN_TIME2;
}

/*
 * NAME :		patchCreateGame_Hook
 * 
 * DESCRIPTION :
 * 			Patches create game save settings to save respawn timer.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
u64 patchCreateGame_Hook(void * a0)
{
	// Save respawn timer if not survivor
	if (!GAMESETTINGS_SURVIVOR)
		GAMESETTINGS_RESPAWN_TIME = GAMESETTINGS_RESPAWN_TIME2;

	// Load normal
	return ((u64 (*)(void *))GAMESETTINGS_CREATE_FUNC)(a0);
}

/*
 * NAME :		patchCreateGame
 * 
 * DESCRIPTION :
 * 			Patches create game save settings to save respawn timer.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void patchCreateGame()
{
	// Patch function pointer
	if (GAMESETTINGS_CREATE_PATCH == 0x0C1C2D50)
	{
		GAMESETTINGS_CREATE_PATCH = 0x0C000000 | ((u32)&patchCreateGame_Hook >> 2);
	}
}

/*
 * NAME :		patchFrameSkip
 * 
 * DESCRIPTION :
 * 			Patches frame skip to always be off.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void patchFrameSkip()
{
	int addrValue = FRAME_SKIP_WRITE0;
	
	if (config.disableFramelimiter && addrValue == 0xAF848859)
	{
		FRAME_SKIP_WRITE0 = 0;
		FRAME_SKIP = 0;
	}
	else if (!config.disableFramelimiter && addrValue == 0)
	{
		FRAME_SKIP_WRITE0 = 0xAF848859;
	}
}

/*
 * NAME :		patchWeaponShotNetSendFlag
 * 
 * DESCRIPTION :
 * 			Patches weapon shot to be sent over TCP instead of UDP.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void patchWeaponShotNetSendFlag(void)
{
	u32* ptr = (u32*)0x00627AB4;
	if (*ptr == 0x906407F8) {
		// change to reliable
		*ptr = 0x24040000 | 0x40;

		// get rid of additional 3 packets sent
		// since its reliable we don't need redundancy
		*(u32*)0x0060F474 = 0;
		*(u32*)0x0060F4C4 = 0;
	}
}

/*
 * NAME :		runGameStartMessager
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
void runGameStartMessager(void)
{
	GameSettings * gameSettings = gameGetSettings();
	if (!gameSettings)
		return;

	// in staging
	if (uiGetActive() == UI_ID_GAME_LOBBY)
	{
		// check if game started
		if (!sentGameStart && gameSettings->GameLoadStartTime > 0)
		{
			// check if host
			if (*(u8*)0x00172170 == 0)
			{
				netSendCustomAppMessage(netGetLobbyServerConnection(), CUSTOM_MSG_ID_GAME_LOBBY_STARTED, 0, gameSettings);
			}
			
			sentGameStart = 1;
		}
	}
	else
	{
		sentGameStart = 0;
	}
}

int checkStateCondition(PlayerStateCondition_t * condition, int localState, int remoteState)
{
	switch (condition->Type)
	{
		case PLAYERSTATECONDITION_REMOTE_EQUALS: // check remote is and local isn't
		{
			return condition->StateId == remoteState;
		}
		case PLAYERSTATECONDITION_LOCAL_EQUALS: // check local is and remote isn't
		{
			return condition->StateId == localState;
		}
		case PLAYERSTATECONDITION_LOCAL_OR_REMOTE_EQUALS: // check local or remote is
		{
			return condition->StateId == remoteState || condition->StateId == localState;
		}
	}

	return 0;
}

/*
 * NAME :		runPlayerStateSync
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
void runPlayerStateSync(void)
{
	const int stateForceCount = sizeof(stateForceRemoteConditions) / sizeof(PlayerStateCondition_t);
	const int stateSkipCount = sizeof(stateSkipRemoteConditions) / sizeof(PlayerStateCondition_t);

	Player ** players = playerGetAll();
	int i,j;

	if (!gameIsIn() || !config.enablePlayerStateSync)
		return;

	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player* p = players[i];
		if (p && !playerIsLocal(p))
		{
			// get remote state
			int localState = p->PlayerState;
			int remoteState = *(int*)((u32)p + 0x3a80);

			// force onto local state
			PlayerVTable* vtable = playerGetVTable(p);
			if (vtable && remoteState != localState)
			{
				int pStateTimer = *(int*)((u32)p + 0x25e0);
				int skip = 0;

				// iterate each condition
				// if one is true, skip to the next player
				for (j = 0; j < stateSkipCount; ++j)
				{
					const PlayerStateCondition_t* condition = &stateSkipRemoteConditions[j];
					if (pStateTimer >= condition->TimeSince)
					{
						if (checkStateCondition(condition, localState, remoteState))
						{
							DPRINTF("%d skipping remote player %08x (%d) state (%d) timer:%d\n", j, (u32)p, p->PlayerId, remoteState, pStateTimer);
							skip = 1;
							break;
						}
					}
				}

				// go to next player
				if (skip)
					continue;

				// iterate each condition
				// if one is true, then force the remote state onto the local player
				for (j = 0; j < stateForceCount; ++j)
				{
					const PlayerStateCondition_t* condition = &stateForceRemoteConditions[j];
					if (pStateTimer >= condition->TimeSince)
					{
						if (checkStateCondition(condition, localState, remoteState))
						{
							DPRINTF("%d changing remote player %08x (%d) state to %d timer:%d\n", j, (u32)p, p->PlayerId, remoteState, pStateTimer);
							vtable->UpdateState(p, remoteState, 1, 1, 1);
							break;
						}
					}
				}
			}
		}
	}
}

/*
 * NAME :		onGameStartMenuBack
 * 
 * DESCRIPTION :
 * 			Called when the user selects 'Back' in the in game start menu
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void onGameStartMenuBack(long a0)
{
	// call start menu back callback
	((void (*)(long))0x00560E30)(a0);

	// open config
	configMenuEnable();
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
	GameSettings * gamesettings = gameGetSettings();

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
				if (gameIsIn())
				{
					// Check if the game hasn't ended
					// We also give the module a second after the game has ended to
					// do some end game logic
					if (!gameHasEnded() || gameGetTime() < (gameGetFinishedExitTime() + TIME_SECOND))
					{
						// Invoke module
						if (module->GameEntrypoint)
							module->GameEntrypoint(module);
					}
				}
				else
				{
					// Invoke lobby module if still active
					if (module->LobbyEntrypoint)
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
			if (!gameIsIn() && module->LobbyEntrypoint)
			{
				module->LobbyEntrypoint(module);
			}
		}

		++module;
	}
}

/*
 * NAME :		onSetTeams
 * 
 * DESCRIPTION :
 * 			Called when the server requests the client to change the lobby's teams.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int onSetTeams(void * connection, void * data)
{
	int i, j;
  ChangeTeamRequest_t request;
	u32 seed;
	char teamByClientId[GAME_MAX_PLAYERS];

	// move message payload into local
	memcpy(&request, data, sizeof(ChangeTeamRequest_t));

	// move seed
	memcpy(&seed, &request.Seed, 4);

	//
	memset(teamByClientId, -1, sizeof(teamByClientId));

#if DEBUG
	printf("pool size: %d\npool: ", request.PoolSize);
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		printf("%d=%d,", i, request.Pool[i]);
	printf("\n");
#endif

	// get game settings
	GameSettings* gameSettings = gameGetSettings();
	if (gameSettings)
	{
		for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		{
			int clientId = gameSettings->PlayerClients[i];
			if (clientId >= 0)
			{
				int teamId = teamByClientId[clientId];
				if (teamId < 0)
				{
					if (request.PoolSize == 0)
					{
						teamId = 0;
					}
					else
					{
						// psuedo random
						sha1(&seed, 4, &seed, 4);

						// get pool index from rng
						int teamPoolIndex = seed % request.PoolSize;
						DPRINTF("pool info pid:%d poolIndex:%d poolSize:%d\n", i, teamPoolIndex, request.PoolSize);

						// set team
						teamId = request.Pool[teamPoolIndex];

						// remove element from pool
						if (request.PoolSize > 0)
						{
							for (j = teamPoolIndex+1; j < request.PoolSize; ++j)
								request.Pool[j-1] = request.Pool[j];
							request.PoolSize -= 1;
						}
					}

					// set client id team
					teamByClientId[clientId] = teamId;
				}

				// set team
				DPRINTF("setting pid:%d to %d\n", i, teamId);
				gameSettings->PlayerTeams[i] = teamId;
			}
		}
	}

	return sizeof(ChangeTeamRequest_t);
}

/*
 * NAME :		onOnlineMenu
 * 
 * DESCRIPTION :
 * 			Called every ui update in menus.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void onOnlineMenu(void)
{
	// call normal draw routine
	((void (*)(void))0x00707F28)();
	
	// 
	lastMenuInvokedTime = gameGetTime();

	//
	if (!hasInitialized)
	{
		padEnableInput();
		hasInitialized = 1;
	}

	// 
	if (hasInitialized == 1 && uiGetActive() == UI_ID_ONLINE_MAIN_MENU)
	{
		uiShowOkDialog("System", "Patch has been successfully loaded.");
		hasInitialized = 2;
	}

	// map loader
	onMapLoaderOnlineMenu();

	// settings
	onConfigOnlineMenu();
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

	// auto enable pad input to prevent freezing when popup shows
	if (lastMenuInvokedTime > 0 && gameGetTime() - lastMenuInvokedTime > TIME_SECOND)
	{
		padEnableInput();
		lastMenuInvokedTime = 0;
	}

	// invoke exception display installer
	if (*(u32*)EXCEPTION_DISPLAY_ADDR != 0)
	{
		if (!hasInstalledExceptionHandler)
		{
			((void (*)(void))EXCEPTION_DISPLAY_ADDR)();
			hasInstalledExceptionHandler = 1;
		}
		
		// change display to match progressive scan resolution
		if (IS_PROGRESSIVE_SCAN)
		{
			*(u16*)(EXCEPTION_DISPLAY_ADDR + 0x9F4) = 0x0083;
			*(u16*)(EXCEPTION_DISPLAY_ADDR + 0x9F8) = 0x210E;
		}
		else
		{
			*(u16*)(EXCEPTION_DISPLAY_ADDR + 0x9F4) = 0x0183;
			*(u16*)(EXCEPTION_DISPLAY_ADDR + 0x9F8) = 0x2278;
		}
	}

	// install net handlers
	netInstallCustomMsgHandler(CUSTOM_MSG_ID_SERVER_REQUEST_TEAM_CHANGE, &onSetTeams);

	// Run map loader
	runMapLoader();

	// Run game start messager
	runGameStartMessager();

	// Run sync player state
	runPlayerStateSync();

	// Patch camera speed
	patchCameraSpeed();

	// Patch announcements
	patchAnnouncements();

	// Patch create game settings load
	patchGameSettingsLoad();

	// Patch populate create game
	patchPopulateCreateGame();

	// Patch save create game settings
	patchCreateGame();

	// Patch frame skip
	patchFrameSkip();

	// Patch weapon shot to be sent reliably
	//patchWeaponShotNetSendFlag();

	// in game stuff
	if (gameIsIn())
	{
		// close config menu on transition to lobby
		if (lastGameState != 1)
			configMenuDisable();

		//
		if (!hasInitialized)
		{
			DPRINTF("patch loaded\n");
			hasInitialized = 1;
		}

		// Hook game start menu back callback
		if (*(u32*)0x003106a0 == 0x00560E30)
		{
			*(u32*)0x003106a0  = &onGameStartMenuBack;
		}

		// patch lod
		if (*(u32*)0x005930B8 == 0x02C3B020)
		{
			*(u32*)0x005930B8 = 0x08000000 | ((u32)&_correctTieLod >> 2);
		}

		// correct lod
		switch (config.levelOfDetail)
		{
			case 0: // low
			{
				_lodScale = 0.2;
				SHRUB_RENDER_DISTANCE = 50;
				*DRAW_SHADOW_FUNC = 0x03E00008;
				*(DRAW_SHADOW_FUNC + 1) = 0;
				break;
			}
			case 1: // normal
			{
				_lodScale = 1.0;
				SHRUB_RENDER_DISTANCE = 500;
				*DRAW_SHADOW_FUNC = 0x27BDFF90;
				*(DRAW_SHADOW_FUNC + 1) = 0xFFB30038;
				break;
			}
			case 2: // high
			{
				_lodScale = 10.0;
				SHRUB_RENDER_DISTANCE = 5000;
				*DRAW_SHADOW_FUNC = 0x27BDFF90;
				*(DRAW_SHADOW_FUNC + 1) = 0xFFB30038;
				break;
			}
		}

		// patch start menu back button text
		*(u32*)0x002AC15C = (u32)patchConfigStr;

		// trigger config menu update
		onConfigGameMenu();

		lastGameState = 1;
	}
	else
	{
		// Hook menu loop
		if (*(u32*)0x00594CBC == 0)
			*(u32*)0x00594CB8 = 0x0C000000 | ((u32)(&onOnlineMenu) / 4);

		// close config menu on transition to lobby
		if (lastGameState != 0)
			configMenuDisable();

		lastGameState = 0;
	}

	// Process game modules
	processGameModules();

	// Process spectate
	if (config.enableSpectate)
		processSpectate();

	// Call this last
	dlPostUpdate();

	return 0;
}
