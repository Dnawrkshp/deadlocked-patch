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

#include "stdio.h"
#include "math.h"
#include "math3d.h"
#include "time.h"
#include "module.h"
#include "game.h"
#include "gamesettings.h"
#include "player.h"
#include "cheats.h"
#include "sha1.h"

typedef struct MobyDef
{
	float ScaleHorizontal;
	float ScaleVertical;
	float ObjectScale;

	short MobyId;
} MobyDef;

/*
 *
 */
int Initialized = 0;

/*
 *
 */
int SpawnRate = TIME_SECOND * 4;

int LastSpawn = 0;

float LastTheta = 0;
float WaterRaiseRate = 0.1 * (1 / 60.0);
float WaterHeight = 0;
int NextItem = 0;

int MobyCount = 0;

Moby * WaterMoby = NULL;

/*
 *
 */
u16 shaBuffer;

/*
 *
 */
VECTOR StartUNK_80 = {
	0.00514222,
	-0.0396723,
	62013.9,
	62013.9
};

VECTOR CurrentPosition = {
	440,
	205,
	106.1,
	0
};

int MobyDefsCount = 3;
MobyDef MobyDefs[] = {
	{ 3, 0.5, 0.5, MOBY_ID_BETA_BOX },
	{ 5, 0.85, 1, MOBY_ID_SARATHOS_BRIDGE },
	{ 5, 0.85, 1, MOBY_ID_OTHER_PART_FOR_SARATHOS_BRIDGE }
};

float RandomRange(float min, float max)
{
	// Generate our rng seed from SHA1 of spawn seed
	sha1(&shaBuffer, 2, (void*)&shaBuffer, 2);

	float value = ((float)shaBuffer / 0xFFFF);

	return (value + min) * (max - min);
}

short RandomRangeShort(short min, short max)
{
	// Generate our rng seed from SHA1 of spawn seed
	sha1(&shaBuffer, 2, (void*)&shaBuffer, 2);

	return (shaBuffer % (max - min)) + min;
}

Moby * spawn(MobyDef * def, VECTOR position, VECTOR rotation, float scale)
{
	Moby * sourceBox;

	// Spawn box so we know the correct model and collision pointers
	sourceBox = spawnMoby(def->MobyId, 0);

	// 
	position[3] = sourceBox->Position[3];
	vector_copy(sourceBox->Position, position);

	//
	vector_copy(sourceBox->Rotation, rotation);

	sourceBox->UNK_30 = 0xFF;
	sourceBox->UNK_31 = 0x01;
	sourceBox->RenderDistance = 0x0080;
	sourceBox->Opacity = 0x80;
	sourceBox->UNK_20[0] = 1;

	sourceBox->UNK_B8 = 1;
	sourceBox->Scale = (float)0.11 * scale * def->ObjectScale;
	sourceBox->UNK_38[0] = 2;
	sourceBox->UNK_38[1] = 2;
	sourceBox->ExtraPropertiesPointer = 0;

	// For this model the vector here is copied to 0x80 in the moby
	// This fixes the occlusion bug
	sourceBox->AnimationPointer = StartUNK_80;

	// 
#if DEBUG
	printf("source: %08x\n", (u32)sourceBox);
#endif

	return sourceBox;
}

void spawnTick(void)
{
	int gameTime = getGameTime();
	VECTOR rot;
	float scale;

	if ((gameTime - LastSpawn) > SpawnRate)
	{
		// Generate new random parameters
		scale = RandomRange(1, 2);
		rot[0] = RandomRange(-0.3, 0.3);
		rot[1] = RandomRange(-0.3, 0.3);
		rot[2] = RandomRange(-1, 1);
		MobyDef * currentItem = &MobyDefs[NextItem];

		// Spawn
		spawn(currentItem, CurrentPosition, rot, scale);

		// Determine next object
		NextItem = RandomRangeShort(0, MobyDefsCount);
		MobyDef * nextItem = &MobyDefs[NextItem];

		// Generate next position
		LastTheta = RandomRange(LastTheta - (MATH_PI/2), LastTheta + (MATH_PI/2));
		scale *= (currentItem->ScaleHorizontal + nextItem->ScaleHorizontal) / 2;
		CurrentPosition[0] += scale * cosf(LastTheta);
		CurrentPosition[1] += scale * sinf(LastTheta);
		CurrentPosition[2] += nextItem->ScaleVertical * RandomRange(1.5, 3);

		// 
		LastSpawn = gameTime;

		++MobyCount;
		SpawnRate -= MobyCount * 5;

		if (WaterRaiseRate < 0.1)
			WaterRaiseRate *= 1.1;
	}

	WaterHeight += WaterRaiseRate;

	// Set water
	((float*)WaterMoby->PropertiesPointer)[19] = WaterHeight;

	// Set death barrier
	setDeathHeight(WaterHeight);
}

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
	// 
	GameSettings * gameSettings = getGameSettings();

	// Init seed
	shaBuffer = (short)gameSettings->GameLoadStartTime;

	// Set survivor
	setGameSurvivor(1);

	// get water moby
	WaterMoby = getWaterMoby();
	WaterHeight = ((float*)WaterMoby->PropertiesPointer)[19];

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

	// Spawn tick
	spawnTick();

	// Fix health
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		if (!p || p->Health <= 0)
			continue;

		if (p->PlayerPosition[2] <= (WaterHeight - 0.5))
			p->Health = 0;
		else
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
