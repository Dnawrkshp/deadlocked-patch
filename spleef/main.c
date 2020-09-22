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

#include "math.h"
#include "vector.h"
#include "time.h"
#include "module.h"
#include "game.h"
#include "gamesettings.h"
#include "player.h"
#include "cheats.h"

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
	Player ** players = getPlayers();

	int i, j;
	int w = 15, h = 15;
	float size = 2.5;
	float radius = 2.5 * ((w + h) / 5);
	VECTOR pos, rot, center;

	// initialize
	vector_copy(pos, StartPos);
	memset(rot, 0, sizeof(rot));

	// Center
	center[0] = StartPos[0] + (size * (w / 2.0));
	center[1] = StartPos[1] + (size * (h / 2.0));
	center[2] = StartPos[2];

	// Set death barrier
	setDeathHeight(StartPos[2] - 10);
	
	// Spawn box so we know the correct model and collision pointers
	Moby * sourceBox = spawnMoby(MOBY_ID_BETA_BOX, 0);

	// 
	pos[3] = sourceBox->Position[3];

	// Spawn boxes
	for (i = 0; i < w; ++i)
	{
		for (j = 0; j < h; ++j)
		{
			Moby * hbMoby = spawnMoby(MOBY_ID_NODE_BOLT_GUARD, 0);
			
			if (hbMoby)
			{
				vector_copy(hbMoby->Position, pos);

				hbMoby->UNK_30 = 0xFF;
				hbMoby->UNK_31 = 0x01;
				hbMoby->RenderDistance = 0x0080;
				hbMoby->Opacity = 0x80;
				hbMoby->UNK_20[0] = 1;

				hbMoby->UNK_B8 = 1;
				hbMoby->Scale = 0.11;
				hbMoby->UNK_38[0] = 2;
				hbMoby->UNK_38[1] = 2;
				hbMoby->ExtraPropertiesPointer = 0;

				// Copy from source box
				hbMoby->ModelPointer = sourceBox->ModelPointer;
				hbMoby->CollisionPointer = sourceBox->CollisionPointer;
				hbMoby->UNK_20[2] = sourceBox->UNK_20[2];
				//hbMoby->UNK_80[3] = hbMoby->UNK_80[2] = hbMoby->UNK_80[1] = hbMoby->UNK_80[0] = 40000; // sourceBox->UNK_80[3] * 3;
			}

			pos[1] += size;
		}

		pos[0] += size;
		pos[1] = StartPos[1];
	}



	// 
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		if (!p)
			continue;

		float theta = (p->PlayerId / (float)GAME_MAX_PLAYERS) * 2.0 * MATH_PI;
		while (theta > MATH_TAU)
			theta -= MATH_PI;

		pos[0] = center[0] + (cosf(theta) * radius);
		pos[1] = center[1] + (sinf(theta) * radius);
		pos[2] = center[2] + 30;

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
	GameSettings * gameSettings = getGameSettings();
	Player ** players = getPlayers();
	int i;

	// Ensure in game
	if (!gameSettings || !isInGame())
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