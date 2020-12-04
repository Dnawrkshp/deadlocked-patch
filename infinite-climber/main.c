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
int BranchRate = TIME_SECOND * 30;
int LastSpawn = 0;
float WaterRaiseRate = 0.1 * (1 / 60.0);
float WaterHeight = 0;
int MobyCount = 0;
Moby * WaterMoby = NULL;


struct ClimbChain
{
	int Active;
	int NextItem;
	float LastTheta;
	int LastBranch;
	VECTOR CurrentPosition;
} Chains[4];

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

VECTOR StartPos = {
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
	sourceBox->Opacity = 0x7E;
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

struct ClimbChain * GetFreeChain(void)
{
	int i = 0;
	for (; i < 4; ++i)
		if (!Chains[i].Active)
			return &Chains[i];

	return NULL;
}

void DestroyOld(void)
{
	Moby ** mobies = getLoadedMobies();
	Moby * moby;

	while ((moby = *mobies++))
	{
		if (moby->Opacity == 0x7E)
		{
			if (moby->Position[2] < WaterHeight)
			{
				mobyDestroy(moby);
				moby->Opacity = 0x7F;
			}
		}
	}
}

void GenerateNext(struct ClimbChain * chain, MobyDef * currentItem, float scale)
{
	// Determine next object
	chain->NextItem = RandomRangeShort(0, MobyDefsCount);
	MobyDef * nextItem = &MobyDefs[chain->NextItem];

	// Adjust scale by current item
	if (currentItem)
		scale *= (currentItem->ScaleHorizontal + nextItem->ScaleHorizontal) / 2;

	// Generate next position
	chain->LastTheta = clampAngle(RandomRange(chain->LastTheta - (MATH_PI/4), chain->LastTheta + (MATH_PI/4)));
	chain->CurrentPosition[0] += scale * cosf(chain->LastTheta);
	chain->CurrentPosition[1] += scale * sinf(chain->LastTheta);
	chain->CurrentPosition[2] += nextItem->ScaleVertical * RandomRange(1.5, 3);
}

void spawnTick(void)
{
	int gameTime = getGameTime();
	int chainIndex = 0;
	VECTOR rot;
	float scale;

	if ((gameTime - LastSpawn) > SpawnRate)
	{
		// Destroy submerged objects
		DestroyOld();

		// 
		for (chainIndex = 0; chainIndex < 4; ++chainIndex)
		{
			struct ClimbChain * chain = &Chains[chainIndex];
			if (!chain->Active)
				continue;

			// Generate new random parameters
			scale = RandomRange(1, 2);
			rot[0] = RandomRange(-0.3, 0.3);
			rot[1] = RandomRange(-0.3, 0.3);
			rot[2] = RandomRange(-1, 1);
			MobyDef * currentItem = &MobyDefs[chain->NextItem];

			// Spawn
			spawn(currentItem, chain->CurrentPosition, rot, scale);

			// Branch
			if ((gameTime - chain->LastBranch) > BranchRate)
			{
				struct ClimbChain * branchChain = GetFreeChain();
				if (branchChain)
				{
					branchChain->Active = 1;
					branchChain->LastBranch = gameTime;
					branchChain->LastTheta = MATH_PI + chain->LastTheta;
					vector_copy(branchChain->CurrentPosition, chain->CurrentPosition);

					// Determine next object
					GenerateNext(branchChain, currentItem, scale);
				}

				chain->LastBranch = gameTime;
			}

			// Determine next object
			GenerateNext(chain, currentItem, scale);
		}


		// 
		LastSpawn = gameTime;

		++MobyCount;
		if (SpawnRate > (TIME_SECOND * 1))
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
const int testCount = 20;
const float testRingCount = 30;
int testIndex = 0;
int testOffset = 0;
int testLastTime = 0;
Moby* mobies[20];

void testTick(void)
{
	int gameTime = getGameTime();
	VECTOR pos;

	if ((gameTime - testLastTime) > (TIME_SECOND * 0.1))
	{
		Moby * item = mobies[testIndex];
		if (item)
		{
			mobyDestroy(item);
		}

		item = spawnMoby(MOBY_ID_BETA_BOX, 0);
		
		float theta = ((float)(testOffset% 30) / (float)testRingCount) * MATH_TAU;
		pos[0] = CurrentPosition[0] + 5 * cosf(theta);
		pos[1] = CurrentPosition[1] + 5 * sinf(theta);
		pos[2] = CurrentPosition[2] + 1;
		vector_copy(item->Position, pos);

		item->Rotation[2] = theta;
		
		item->UNK_30 = 0xFF;
		item->UNK_31 = 0x01;
		item->RenderDistance = 0x0080;
		item->Opacity = 0x80;
		item->UNK_20[0] = 1;

		item->UNK_B8 = 1;
		item->Scale = (float)0.05;
		item->UNK_38[0] = 2;
		item->UNK_38[1] = 2;
		item->ExtraPropertiesPointer = 0;

		mobies[testIndex] = item;
		++testIndex;
		++testOffset;
		if (testIndex >= testCount)
			testIndex = 0;
		testLastTime = gameTime;
	}
}
*/

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

	memset(Chains, 0, sizeof(Chains));

	Chains[0].Active = 1;
	vector_copy(Chains[0].CurrentPosition, StartPos);
	Chains[0].LastBranch = getGameTime();

	// 
	//memset(mobies, 0, sizeof(mobies));
	//testIndex = 0;

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

	//testTick();

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
