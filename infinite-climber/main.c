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
#include <libdl/game.h>
#include <libdl/gamesettings.h>
#include <libdl/player.h>
#include <libdl/cheats.h>
#include <libdl/sha1.h>
#include <libdl/map.h>
#include <libdl/dialog.h>
#include <libdl/ui.h>
#include "module.h"

#define MAX_MAP_MOBY_DEFS		(10)
#define MAX_WATER_RATE			(0.03)
#define MAX_SPAWN_RATE			(TIME_SECOND * 0.5)

/*
 * When non-zero, it refreshes the in-game scoreboard.
 */
#define GAME_SCOREBOARD_REFRESH_FLAG        (*(u32*)0x00310248)

/*
 * Target scoreboard value.
 */
#define GAME_SCOREBOARD_TARGET              (*(u32*)0x002FA084)

/*
 * Collection of scoreboard items.
 */
#define GAME_SCOREBOARD_ARRAY               ((ScoreboardItem**)0x002FA04C)

/*
 * Number of items in the scoreboard.
 */
#define GAME_SCOREBOARD_ITEM_COUNT          (*(u32*)0x002F9FCC)


typedef struct MobyDef
{
	float ScaleHorizontal;
	float ScaleVertical;
	float ObjectScale;

	short MobyId;
	short MapMask;
} MobyDef;

struct State
{
	int TimePlayerDied[GAME_MAX_PLAYERS];
	float PlayerFinalHeight[GAME_MAX_PLAYERS];
	float PlayerBestHeight[GAME_MAX_PLAYERS];
	int StartTime;
	int EndTime;
	float StartHeight;
} State;

/*
 *
 */
int Initialized = 0;

/*
 *
 */
int SpawnRate = TIME_SECOND * 3;
int BranchRate = TIME_SECOND * 30;
int LastSpawn = 0;
float WaterRaiseRate = 0.1 * (1 / 60.0);
float WaterHeight = 0;
int MobyCount = 0;
Moby * WaterMoby = NULL;
int BranchDialogs[] = { DIALOG_ID_CLANK_YOU_HAVE_A_CHOICE_OF_2_PATHS, DIALOG_ID_DALLAS_WOAH_THIS_IS_GETTING_INTERESTING, DIALOG_ID_DALLAS_KICKING_PROVERBIAL_BUTT_IDK_WHAT_THAT_MEANS, DIALOG_ID_DALLAS_DARKSTAR_TIGHT_SPOTS_BEFORE  };
int StartDialogs[] = { DIALOG_ID_TEAM_DEADSTAR, DIALOG_ID_DALLAS_SHOWTIME, DIALOG_ID_DALLAS_RATCHET_LAST_WILL_AND_TESTAMENT, DIALOG_ID_DALLAS_WHO_PACKED_YOUR_PARACHUTE, };
int IncrementalDialogs[] = { DIALOG_ID_PLEASE_TAKE_YOUR_TIME, DIALOG_ID_DALLAS_SHOWOFF, DIALOG_ID_JUANITA_MORON, DIALOG_ID_JUANITA_I_CANT_BEAR_TO_LOOK_YES_I_CAN, DIALOG_ID_DALLAS_CARNAGE_LAST_RELATIONSHIP, DIALOG_ID_DALLAS_LOOK_AT_THAT_LITTLE_GUY_GO,  };


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

int MapMobyDefsCount = 0;
MobyDef * MapMobyDefs[MAX_MAP_MOBY_DEFS];

MobyDef MobyDefs[] = {
	
	//{ 5, 0.85, 1, MOBY_ID_PART_CATACROM_BRIDGE, MAP_MASK_CATACROM },
	{ 5, 0.8, 1, MOBY_ID_SARATHOS_BRIDGE, MAP_MASK_BATTLEDOME },

	{ 5, 0.85, 1, MOBY_ID_SARATHOS_BRIDGE, MAP_MASK_SARATHOS },
	{ 5, 0.85, 1, MOBY_ID_OTHER_PART_FOR_SARATHOS_BRIDGE, MAP_MASK_SARATHOS },

	{ 5, 0.8, 1, MOBY_ID_DARK_CATHEDRAL_SECRET_PLATFORM, MAP_MASK_DC },

	
	//{ 5, 0.8, 0.5, MOBY_ID_TURRET_SHIELD_UPGRADE, MAP_MASK_ALL }, //this freezes ps2, need to fix sometime
	{ 3, 0.5, 0.5, MOBY_ID_BETA_BOX, MAP_MASK_ALL },
	{ 5, 0.85, 1, MOBY_ID_VEHICLE_PAD, MAP_MASK_ALL },
	{ 3, 0.8, 0.5, MOBY_ID_TELEPORT_PAD, MAP_MASK_ALL },
	//{ 3, 0.8, 0.5, MOBY_ID_PICKUP_PAD, MAP_MASK_ALL }
};


/*
 *
 */
ScoreboardItem PlayerScores[GAME_MAX_PLAYERS];

/*
 *
 */
ScoreboardItem * SortedPlayerScores[GAME_MAX_PLAYERS];


/*
 * NAME :		sortScoreboard
 * 
 * DESCRIPTION :
 * 			Sorts the scoreboard by value.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 		player			:		Target player's player object.
 * 		playerState 	:		Target player's gun game state.
 * 		playerWepStats 	:		Target player's weapon stats.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void sortScoreboard(int dontLockLocal)
{
	int i = 0;
	int j = 0;

	// bubble sort
	for (j = GAME_MAX_PLAYERS - 1; j > 0; --j)
	{
		for (i = 0; i < j; ++i)
		{
			// Swap
			if (SortedPlayerScores[i]->TeamId < 0 ||
				((dontLockLocal || SortedPlayerScores[i]->UNK != 1) &&
				 (SortedPlayerScores[i]->Value < SortedPlayerScores[i+1]->Value || 
				 SortedPlayerScores[i+1]->UNK == 1)))
			{
				ScoreboardItem * temp = SortedPlayerScores[i];
				SortedPlayerScores[i] = SortedPlayerScores[i+1];
				SortedPlayerScores[i+1] = temp;
			}
		}
	}
}

/*
 * NAME :		updateScoreboard
 * 
 * DESCRIPTION :
 * 			Updates the in game scoreboard.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void updateScoreboard(void)
{
	int i;

	// Correct scoreboard
	for (i = 0; i < GAME_SCOREBOARD_ITEM_COUNT; ++i)
	{
		// Force scoreboard to custom scoreboard values
		if (GAME_SCOREBOARD_ARRAY[i] != SortedPlayerScores[i])
		{
			GAME_SCOREBOARD_ARRAY[i] = SortedPlayerScores[i];
			GAME_SCOREBOARD_REFRESH_FLAG = 1;
		}
	}
}

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

Moby * spawnWithPVars(int mobyId)
{
	switch (mobyId)
	{
		case MOBY_ID_VEHICLE_PAD: return mobySpawn(mobyId, 0x60);
		case MOBY_ID_PICKUP_PAD: return mobySpawn(mobyId, 0x90);
		case MOBY_ID_TELEPORT_PAD: return mobySpawn(mobyId, 0xD0);
		case MOBY_ID_TURRET_SHIELD_UPGRADE: return mobySpawn(mobyId, 0xD0);
		default: return mobySpawn(mobyId, 0);
	}
}

Moby * spawn(MobyDef * def, VECTOR position, VECTOR rotation, float scale)
{
	Moby * sourceBox;

	// Spawn box so we know the correct model and collision pointers
	sourceBox = spawnWithPVars(def->MobyId);
	if (!sourceBox)
		return 0;

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

	sourceBox->Scale = (float)0.11 * scale * def->ObjectScale;
	sourceBox->UNK_38[0] = 2;
	sourceBox->UNK_38[1] = 2;
	sourceBox->GuberMoby = 0;

	// For this model the vector here is copied to 0x80 in the moby
	// This fixes the occlusion bug
	sourceBox->AnimationPointer = StartUNK_80;

	// 
	DPRINTF("source: %08x\n", (u32)sourceBox);

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
	Moby * moby = mobyGetFirst();

	while (moby)
	{
		if (moby->Opacity == 0x7E)
		{
			if (moby->Position[2] < WaterHeight)
			{
				mobyDestroy(moby);
				moby->Opacity = 0x7F;
				--MobyCount;
			}
		}

		moby = moby->NextMoby;
	}
}

void GenerateNext(struct ClimbChain * chain, MobyDef * currentItem, float scale)
{
	// Determine next object
	chain->NextItem = RandomRangeShort(0, MapMobyDefsCount);
	MobyDef * nextItem = MapMobyDefs[chain->NextItem];

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
	int gameTime = gameGetTime();
	int chainIndex = 0;
	VECTOR rot;
	float scale;

	if ((gameTime - LastSpawn) > SpawnRate)
	{
		// Destroy submerged objects
		DestroyOld();

		// 
		for (chainIndex = 0; chainIndex < 3; ++chainIndex)
		{
			struct ClimbChain * chain = &Chains[chainIndex];
			if (!chain->Active)
				continue;

			// Generate new random parameters
			scale = RandomRange(1, 2);
			rot[0] = RandomRange(-0.3, 0.3);
			rot[1] = RandomRange(-0.3, 0.3);
			rot[2] = RandomRange(-1, 1);
			MobyDef * currentItem = MapMobyDefs[chain->NextItem];

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
					dialogPlaySound(BranchDialogs[RandomRangeShort(0, sizeof(BranchDialogs)/sizeof(int)-1)], 0);
				}

				chain->LastBranch = gameTime;
			}

			// Determine next object
			GenerateNext(chain, currentItem, scale);
		}

		if(MobyCount == 0)
			dialogPlaySound(StartDialogs[RandomRangeShort(0, sizeof(BranchDialogs)/sizeof(int)-1)], 0);
		else if(MobyCount % 20 == 0)
			dialogPlaySound(IncrementalDialogs[RandomRangeShort(0, sizeof(BranchDialogs)/sizeof(int)-1)], 0);


		// 
		LastSpawn = gameTime;

		++MobyCount;
		if (SpawnRate > MAX_SPAWN_RATE)
			SpawnRate -= MobyCount * 6.5;
		else
			SpawnRate = MAX_SPAWN_RATE;

		if (WaterRaiseRate < MAX_WATER_RATE)
			WaterRaiseRate *= 1.1;
		else
			WaterRaiseRate = MAX_WATER_RATE;
	}

	WaterHeight += WaterRaiseRate;

	// Set water
	((float*)WaterMoby->PropertiesPointer)[19] = WaterHeight;

	// Set death barrier
	gameSetDeathHeight(WaterHeight);
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
	VECTOR startPos;
	int i;

	// 
	GameSettings * gameSettings = gameGetSettings();
	GameOptions * gameOptions = gameGetOptions();
	Player ** players = playerGetAll();

	// Init seed
	shaBuffer = (short)gameSettings->GameLoadStartTime;

	// Set survivor
	gameOptions->GameFlags.MultiplayerGameFlags.Survivor = 1;

	// Disable respawn
	gameOptions->GameFlags.MultiplayerGameFlags.RespawnTime = 0xFF;

	// get water moby
	WaterMoby = mobyGetFirst(); // big assumption here, could be a problem
	WaterHeight = ((float*)WaterMoby->PropertiesPointer)[19];

	// 
	switch (gameSettings->GameLevel)
	{
		case MAP_ID_BATTLEDOME:
		{
			startPos[0] = 846;
			startPos[1] = 556.5;
			startPos[2] = 540;
			break;
		}
		case MAP_ID_CATACROM:
		{
			startPos[0] = 350.3;
			startPos[1] = 254;
			startPos[2] = 63.1;
			break;
		}
		case MAP_ID_SARATHOS:
		{
			startPos[0] = 440;
			startPos[1] = 205;
			startPos[2] = 106.1;
			break;
		}
		case MAP_ID_DC:
		{
			startPos[0] = 503;
			startPos[1] = 493;
			startPos[2] = 641.3;
			break;
		}
		case MAP_ID_SHAAR:
		{
			startPos[0] = 484.5;
			startPos[1] = 569.5;
			startPos[2] = 509.3;
			break;
		}
		case MAP_ID_VALIX:
		{
			startPos[0] = 311.5;
			startPos[1] = 488.5;
			startPos[2] = 331;
			break;
		}
		case MAP_ID_MF:
		{
			startPos[0] = 410;
			startPos[1] = 598;
			startPos[2] = 434.5;
			break;
		}
		case MAP_ID_TORVAL:
		{
			startPos[0] = 300;
			startPos[1] = 371;
			startPos[2] = 106;
			break;
		}
		case MAP_ID_TEMPUS:
		{
			startPos[0] = 462;
			startPos[1] = 512;
			startPos[2] = 102.5;
			break;
		}
		case MAP_ID_MARAXUS:
		{
			startPos[0] = 501;
			startPos[1] = 766.5;
			startPos[2] = 103.5;
			break;
		}
		case MAP_ID_GS:
		{
			startPos[0] = 732;
			startPos[1] = 565;
			startPos[2] = 101.5;
			break;
		}
	}

	// 
	State.StartTime = gameGetTime();
	State.StartHeight = startPos[2];
	State.EndTime = 0;

	// 
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		State.PlayerFinalHeight[i] = 0;
		State.PlayerBestHeight[i] = 0;
		State.TimePlayerDied[i] = 0;
	}
	
	// Initialize scoreboard
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		PlayerScores[i].TeamId = p ? i : -1;
		PlayerScores[i].UNK = playerIsLocal(p);
		PlayerScores[i].Value = 0;
		SortedPlayerScores[i] = &PlayerScores[i];
	}

	//
	sortScoreboard(0);
	updateScoreboard();

	// Populate moby defs
	int mapIdMask = mapIdToMask(gameSettings->GameLevel);
	for (i = 0; i < sizeof(MobyDefs)/sizeof(MobyDef); ++i)
	{
		if (MapMobyDefsCount >= MAX_MAP_MOBY_DEFS)
			break;

		if (mapMaskHasMask(MobyDefs[i].MapMask, mapIdMask))
			MapMobyDefs[MapMobyDefsCount++] = &MobyDefs[i];
	}

	// 
	memset(Chains, 0, sizeof(Chains));
	Chains[0].Active = 1;
	vector_copy(Chains[0].CurrentPosition, startPos);
	Chains[0].LastBranch = LastSpawn = gameGetTime();


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

	// Spawn tick
	spawnTick();

	// Fix health
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		if (!p || p->Health <= 0)
			continue;

		// set max height
		float height = p->PlayerPosition[2] - State.StartHeight;
		State.PlayerFinalHeight[i] = height;
		if (height > State.PlayerBestHeight[i])
		{
			State.PlayerBestHeight[i] = height;
			PlayerScores[i].Value = (int)height;
			sortScoreboard(0);
			GAME_SCOREBOARD_REFRESH_FLAG = 1;
		}

		// kill or give health
		if (p->PlayerPosition[2] <= (WaterHeight - 0.5))
		{
			p->Health = 0;
			State.TimePlayerDied[i] = gameGetTime();
		}
		else
		{
			p->Health = PLAYER_MAX_HEALTH;
		}
	}

	// 
	updateScoreboard();
}

void setLobbyGameOptions(void)
{
	// deathmatch options
	static char options[] = { 
		0, 0, 				// 0x06 - 0x08
		0, 0, 0, 0, 	// 0x08 - 0x0C
		1, 1, 1, 0,  	// 0x0C - 0x10
		0, 1, 0, 0,		// 0x10 - 0x14
		-1, -1, 0, 1,	// 0x14 - 0x18
	};

	// set game options
	GameOptions * gameOptions = gameGetOptions();
	if (!gameOptions)
		return;
		
	// apply options
	memcpy((void*)&gameOptions->GameFlags.Raw[6], (void*)options, sizeof(options)/sizeof(char));
	gameOptions->GameFlags.MultiplayerGameFlags.Juggernaut = 0;
}

void setEndGameScoreboard(void)
{
	u32 * uiElements = (u32*)(*(u32*)(0x011C7064 + 4*18) + 0xB0);
	GameSettings * gs = gameGetSettings();
	int i;
	char buf[24];

	// reset buf
	memset(buf, 0, sizeof(buf));

	// give more detail to player score
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		PlayerScores[i].Value = (int)(State.PlayerBestHeight[i] * 100);
	}

	// sort scoreboard again
	sortScoreboard(1);

	// names start at 7
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		int pid = SortedPlayerScores[i]->TeamId;
		strncpy((char*)(uiElements[7 + i] + 0x18), gs->PlayerNames[pid], 16);
	}

	// column headers start at 17
	strncpy((char*)(uiElements[18] + 0x60), "DISTANCE", 9);
	strncpy((char*)(uiElements[19] + 0x60), "TIME", 5);

	// rows
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		int pid = SortedPlayerScores[i]->TeamId;
		if (pid >= 0)
		{
			// set distance
			sprintf(buf, "%.2f", SortedPlayerScores[i]->Value / 100.0);
			strncpy((char*)(uiElements[22 + (i*4) + 0] + 0x60), buf, strlen(buf) + 1);

			// set time alive
			int pTime = State.TimePlayerDied[pid] - State.StartTime;
			if (pTime < 0)
				pTime = State.EndTime - State.StartTime;

			sprintf(buf, "%02d:%02d", pTime / TIME_MINUTE, (pTime % TIME_MINUTE) / TIME_SECOND);
			strncpy((char*)(uiElements[22 + (i*4) + 1] + 0x60), buf, strlen(buf) + 1);

		}
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
	int activeId = uiGetActive();
	static int initializedScoreboard = 0;

	// set time ended
	if (Initialized && State.StartTime && !State.EndTime)
		State.EndTime = gameGetTime();

	// scoreboard
	switch (activeId)
	{
		case 0x15C:
		{
			if (initializedScoreboard)
				break;

			setEndGameScoreboard();
			initializedScoreboard = 1;
			break;
		}
		case UI_ID_GAME_LOBBY:
		{
			setLobbyGameOptions();
			break;
		}
	}
}

/*
 * NAME :		loadStart
 * 
 * DESCRIPTION :
 * 			Load logic entrypoint.
 * 
 * NOTES :
 * 			This is called only when the game has finished reading the level from the disc and before it has started processing the data.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void loadStart(void)
{
	
}
