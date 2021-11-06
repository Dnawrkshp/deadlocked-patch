/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Unhooks patch file and spawns menu popup telling user patch is downloading.
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
#include "module.h"
#include <libdl/game.h>
#include <libdl/string.h>
#include <libdl/stdio.h>
#include <libdl/gamesettings.h>
#include <libdl/dialog.h>
#include <libdl/patch.h>
#include <libdl/ui.h>
#include <libdl/graphics.h>

const int patches[][3] = {
	// patch
	{ 0, 0x0072C3FC, 0x0C1CBBDE }, // GAMESETTINGS_LOAD_PATCH
	{ 0, 0x004B882C, 0x00712BF0 }, // GAMESETTINGS_BUILD_PTR
	{ 0, 0x0072E5B4, 0x0C1C2D50 }, // GAMESETTINGS_CREATE_PATCH
	{ -1, 0x01EAAB10, 0x03E00008 }, // GET_MEDIUS_APP_HANDLER_HOOK
	{ -1, 0x00211E64, 0x00000000 }, // net global callbacks ptr
	{ -1, 0x00212164, 0x00000000 }, // dme callback table custom msg handler ptr
	{ -1, 0x00157D38, 0x0C055E68 }, // process level jal
	// maploader
	{ 0, 0x005CFB48, 0x0C058E10 }, // hookLoadAddr
	{ 0, 0x00705554, 0x0C058E02 }, // hookLoadingScreenAddr
	{ -1, 0x00163814, 0x0C058E10 }, // hookLoadCdvdAddr
	{ 0, 0x005CF9B0, 0x0C058E4A }, // hookCheckAddr
	{ -1, 0x00159B20, 0x0C056680 }, // hookTableAddr
	{ -1, 0x00159B20, 0x0C056680 }, // hookTableAddr
	{ 0, 0x007055B4, 0x0C046A7B }, // hook loading screen map name strcpy
	// in game
	{ 1, 0x005930B8, 0x02C3B020 }, // lod patch
};

const int clears[][2] = {
	{ 0x000D0000, 0x00008000 }, // patch
	{ 0x000C8000, 0x00007000 }, // gamerules
	{ 0x000CF000, 0x00001000 }, // module definitions
};

int hasClearedMemory = 0;

/*
 * NAME :		onOnlineMenu
 * 
 * DESCRIPTION :
 * 			Called every ui update.
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
	u32 bgColorDownload = 0x70000000;
	u32 downloadColor = 0x80808080;

	// call normal draw routine
	((void (*)(void))0x00707F28)();

	// only show on main menu
	if (uiGetActive() != UI_ID_ONLINE_MAIN_MENU)
		return;

	// render background
	gfxScreenSpaceBox(0.2, 0.35, 0.6, 0.3, bgColorDownload);

	// flash color
	int gameTime = ((gameGetTime()/100) % 15);
	if (gameTime > 7)
		gameTime = 15 - gameTime;
	downloadColor += 0x101010 * gameTime;

	// render text
	gfxScreenSpaceText(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, 1, 1, downloadColor, "Downloading patch, please wait...", -1, 4);
}

/*
 * NAME :		main
 * 
 * DESCRIPTION :
 * 			Entrypoint.
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
	int i;
	const int patchesSize =  sizeof(patches) / (3 * sizeof(int));
	const int clearsSize =  sizeof(clears) / (2 * sizeof(int));
	int inGame = gameIsIn();

	// unhook patch
	for (i = 0; i < patchesSize; ++i)
	{
		int context = patches[i][0];
		if (context < 0 || context == inGame)
			*(u32*)patches[i][1] = (u32)patches[i][2];
	}

	// clear memory
	if (!hasClearedMemory)
	{
		hasClearedMemory = 1;
		for (i = 0; i < clearsSize; ++i)
		{
			memset((void*)clears[i][0], 0, clears[i][1]);
		}
	}

	// 
	if (inGame)
		return;

	// Hook menu loop
	*(u32*)0x00594CB8 = 0x0C000000 | ((u32)(&onOnlineMenu) / 4);

	// disable pad on online main menu
	if (uiGetActive() == UI_ID_ONLINE_MAIN_MENU)
		padDisableInput();

	return 0;
}
