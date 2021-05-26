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

const int patches[][2] = {
	// patch
	{ 0x0072C3FC, 0x0C1CBBDE }, // GAMESETTINGS_LOAD_PATCH
	{ 0x004B882C, 0x00712BF0 }, // GAMESETTINGS_BUILD_PTR
	{ 0x0072E5B4, 0x0C1C2D50 }, // GAMESETTINGS_CREATE_PATCH
	// maploader
	{ 0x005CFB48, 0x0C058E10 }, // hookLoadAddr
	{ 0x00705554, 0x0C058E02 }, // hookLoadingScreenAddr
	{ 0x00163814, 0x0C058E10 }, // hookLoadCdvdAddr
	{ 0x005CF9B0, 0x0C058E4A }, // hookCheckAddr
	{ 0x00159B20, 0x0C056680 }, // hookTableAddr
	{ 0x00159B20, 0x0C056680 }, // hookTableAddr
};

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
	RECT boxRectDownload = {
		{ 0.2, 0.35 },
		{ 0.8, 0.35 },
		{ 0.2, 0.65 },
		{ 0.8, 0.65 }
	};

	// call normal draw routine
	((void (*)(void))0x00707F28)();

	// only show on main menu
	//if (uiGetActive() != UI_ID_ONLINE_MAIN_MENU)
	//	return;

	// render background
	gfxScreenSpaceBox(&boxRectDownload, bgColorDownload, bgColorDownload, bgColorDownload, bgColorDownload);

	// flash color
	int gameTime = ((gameGetTime()/100) % 15);
	if (gameTime > 7)
		gameTime = 15 - gameTime;
	downloadColor += 0x101010 * gameTime;

	// render text
	gfxScreenSpaceText(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.45, 1, 1, downloadColor, "Downloading patch, please wait...", -1);
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
	const int size =  sizeof(patches) / (2 * sizeof(int));

	// unhook patch
	for (i = 0; i < size; ++i)
	{
		*(u32*)patches[i][0] = (u32)patches[i][1];
	}

	// Hook menu loop
	*(u32*)0x00594CB8 = 0x0C000000 | ((u32)(&onOnlineMenu) / 4);

	return 0;
}
