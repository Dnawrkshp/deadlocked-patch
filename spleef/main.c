/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Infected entrypoint and logic.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#include <tamtypes.h>
#include <string.h>

#include <libdl/stdio.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/time.h>
#include "module.h"
#include <libdl/game.h>
#include <libdl/gamesettings.h>
#include <libdl/player.h>
#include <libdl/cheats.h>

/*
 *
 */
int Initialized = 0;

/*
 * Position that boxes are spawned to.
 */
VECTOR StartPos = {
	400,
	400,
	800,
	0
};

/*
 *
 */
VECTOR StartUNK_80 = {
	0.00514222,
	-0.0396723,
	62013.9,
	62013.9
};

/*
 * NAME :		initialize
 * 
 * DESCRIPTION :
 * 			Initializes the gamemode.
 * 
 * NOTES :
 * 			This is called only once at the start.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void initialize(void)
{
	GameSettings * gameSettings = gameGetSettings();
	GameOptions * gameOptions = gameGetOptions();
	Player ** players = playerGetAll();

	int i, j;
	int w = 15, h = 15;
	float size = 2.5;
	float radius = 2.5 * ((w + h) / 5);
#if DEBUG
	int count = 0;
#endif
	VECTOR pos, rot, center;
	Moby * sourceBox, * hbMoby;

	// initialize
	vector_copy(pos, StartPos);
	memset(rot, 0, sizeof(rot));

	// Center
	center[0] = StartPos[0] + (size * (w / (float)2.0));
	center[1] = StartPos[1] + (size * (h / (float)2.0));
	center[2] = StartPos[2];

	// Set death barrier
	gameSetDeathHeight(StartPos[2] - 10);

	// Set survivor
	gameOptions->GameFlags.MultiplayerGameFlags.Survivor = 1;
	
	// Spawn box so we know the correct model and collision pointers
	sourceBox = mobySpawn(MOBY_ID_BETA_BOX, 0);

	// 
	pos[3] = sourceBox->Position[3];

	// Spawn boxes
	for (i = 0; i < w; ++i)
	{
		for (j = 0; j < h; ++j)
		{
			hbMoby = mobySpawn(MOBY_ID_NODE_BOLT_GUARD, 0);
			
			if (hbMoby)
			{
				vector_copy(hbMoby->Position, pos);

				hbMoby->UNK_30 = 0xFF;
				hbMoby->UNK_31 = 0x01;
				hbMoby->RenderDistance = 0x0080;
				hbMoby->Opacity = 0x80;
				hbMoby->UNK_20[0] = 1;

				hbMoby->UNK_B8 = 1;
				hbMoby->Scale = (float)0.11;
				hbMoby->UNK_38[0] = 2;
				hbMoby->UNK_38[1] = 2;
				hbMoby->GuberMoby = 0;

				// For this model the vector here is copied to 0x80 in the moby
				// This fixes the occlusion bug
				hbMoby->AnimationPointer = StartUNK_80;

				// Copy from source box
				hbMoby->ModelPointer = sourceBox->ModelPointer;
				hbMoby->CollisionPointer = sourceBox->CollisionPointer;
				hbMoby->UNK_20[2] = sourceBox->UNK_20[2];

#if DEBUG
				++count;
#endif
			}

			pos[1] += size;
		}

		pos[0] += size;
		pos[1] = StartPos[1];
	}

	// 
#if DEBUG
	hbMoby->Opacity = 0xFF;
	printf("count: %d, source: %08x, new: %08x\n", count, (u32)sourceBox, (u32)hbMoby);
#endif

	// 
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		if (!p)
			continue;

		float theta = (p->PlayerId / (float)gameSettings->PlayerCount) * (float)2.0 * MATH_PI;
		while (theta > MATH_TAU)
			theta -= MATH_PI;

		pos[0] = center[0] + (cosf(theta) * radius);
		pos[1] = center[1] + (sinf(theta) * radius);
		pos[2] = center[2] + (float)30;

		// 
		rot[2] = theta - MATH_PI;

		// 
		playerSetPosRot(p, pos, rot);
	}

	Initialized = 1;
}


/*
 * NAME :		gameStart
 * 
 * DESCRIPTION :
 * 			Infected game logic entrypoint.
 * 
 * NOTES :
 * 			This is called only when in game.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void gameStart(void)
{
	GameSettings * gameSettings = gameGetSettings();
	Player ** players = playerGetAll();
	int i;

	// Ensure in game
	if (!gameSettings || !gameIsIn())
		return;

	if (!Initialized)
		initialize();

	// 
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		if (!p)
			continue;

		p->Health = PLAYER_MAX_HEALTH;
	}

	return;
}

/*
 * NAME :		lobbyStart
 * 
 * DESCRIPTION :
 * 			Infected lobby logic entrypoint.
 * 
 * NOTES :
 * 			This is called only when in lobby.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void lobbyStart(void)
{

}
