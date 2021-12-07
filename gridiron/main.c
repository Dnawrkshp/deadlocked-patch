/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Handles all gridiron logic.
 * 
 * NOTES :
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#include <tamtypes.h>
#include <string.h>

#include <libdl/time.h>
#include <libdl/game.h>
#include <libdl/gamesettings.h>
#include <libdl/graphics.h>
#include <libdl/player.h>
#include <libdl/weapon.h>
#include <libdl/hud.h>
#include <libdl/cheats.h>
#include <libdl/sha1.h>
#include <libdl/dialog.h>
#include <libdl/team.h>
#include <libdl/stdio.h>
#include <libdl/ui.h>
#include <libdl/guber.h>
#include <libdl/color.h>
#include <libdl/radar.h>
#include <libdl/sound.h>
#include <libdl/net.h>
#include "module.h"
#include "messageid.h"
#include "include/ball.h"

// How long after dropping the ball before you can pick it up again
#define BALL_CARRY_COOLDOWN			(TIME_SECOND * 3)

/*
 *
 */
typedef struct GameplayMobyDef
{
	u32 Size;
	char Mission;
	int UID;
	int Bolts;
	int OClass;
	float Scale;
	float DrawDistance;
	float UpdateDistance;
	short UNK_20;
	short UNK_22;
	short UNK_24;
	short UNK_26;
	float PosX;
	float PosY;
	float PosZ;
	float RotX;
	float RotY;
	float RotZ;
	int Group;
	int IsRooted;
	float RootedDistance;
	short UNK_4C;
	short UNK_4E;
	int PVarIndex;
	short UNK_54;
	short UNK_56;
	int UNK_58;
	int Red;
	int Green;
	int Blue;
	int Light;
	int UNK_6C;
} GameplayMobyDef_t;

/*
 *
 */
typedef struct GridironPlayerState
{
	Player * Player;
	int TimeLastCarrier;
} GridironPlayerState_t;

/*
 *
 */
struct GridironState
{
	int GameOver;
	int IsHost;
	Moby * BallMoby;
	Player * BallCarrier;
	GridironPlayerState_t Players[GAME_MAX_PLAYERS];
	Moby * Base[2];
} GridironState;

/*
 *
 */
int Initialized = 0;
int InitializedTime = 0;

/*
 *
 */
char shaBuffer = 0;

/* 
 * Explosion sound def
 */
SoundDef ExplosionSoundDef =
{
	1000.0,		// MinRange
	1000.0,		// MaxRange
	2000,		// MinVolume
	2000,		// MaxVolume
	0,			// MinPitch
	0,			// MaxPitch
	0,			// Loop
	0x10,		// Flags
	0xF4,		// Index
	3			  // Bank
};



/*
 * Configurable settings
 */

// Where the ball spawns
VECTOR BallSpawnPoint __attribute__((section(".config"))) = { 428.368, 239.646, 107.613, 0 };

void GuberMobyEventHandler(Moby * moby, GuberEvent * event, MobyEventHandler_func eventHandler)
{
	switch (moby->MobyId)
	{
		default:
			DPRINTF("GuberMoby event (%04x) with %08x and %08x, handler=%08x\n", moby->MobyId, (u32)moby, (u32)event, (u32)eventHandler);
			eventHandler(moby, event);
			break;
	}
}

void spawnPlayer(Player * player, VECTOR position)
{
	VECTOR pos, r = {0,0,0,0};
	float theta = (player->PlayerId / (float)GAME_MAX_PLAYERS) * MATH_TAU;

	vector_copy(pos, position);
	pos[0] += cosf(theta) * 2.5;
	pos[1] += sinf(theta) * 2.5;
	r[2] = position[3];

	player->Explode = 0;
	player->Invisible = 0;
	playerRespawn(player);
	playerSetPosRot(player, pos, r);
}

void replaceString(int textId, const char * str)
{
	// Get pointer to game string
	char * strPtr = uiMsgString(textId);
	strncpy(strPtr, str, 32);
}

Moby * spawnExplosion(VECTOR position, float size)
{
	// SpawnMoby_5025
	u128 param_1 = *(u128*)position;
	Moby * moby = ((Moby* (*)(u128, float, int, int, int, int, int, short, short, short, short, short, short,
				short, short, float, float, float, int, Moby *, int, int, int, int, int, int, int, int,
				int, short, Moby *, Moby *, u128)) (0x003c3b38))
				(param_1, size, 0x2, 0x14, 0x10, 0x10, 0x10, 0x10, 0x2, 0, 1, 0, 0,
				0, 0, 0, 0, 2, 0x00080800, 0, 0x00388EF7, 0x000063F7, 0x00407FFFF, 0x000020FF, 0x00008FFF, 0x003064FF, 0x7F60A0FF, 0x280000FF,
				0x003064FF, 0, 0, 0, 0);
				
	soundPlay(&ExplosionSoundDef, 0, moby, 0, 0x400);

	return moby;
}

void drawRoundMessage(const char * message, float scale)
{
	u32 boxColor = 0x20ffffff;
	int fw = gfxGetFontWidth(message, -1, scale);
	float x = 0.5;
	float y = 0.16;
	float p = 0.02;
	float w = (fw / (float)SCREEN_WIDTH) + p;
	float h = (36.0 / SCREEN_HEIGHT) + p;

	// draw container
	gfxScreenSpaceBox(x-(w/2), y-(h/2), w, h, boxColor);

	// draw message
	gfxScreenSpaceText(SCREEN_WIDTH * x, SCREEN_HEIGHT * y, scale, scale * 1.5, 0x80FFFFFF, message, -1, 4);
}

void playTimerTickSound()
{
	((void (*)(Player*, int, int))0x005eb280)((Player*)0x347AA0, 0x3C, 0);
}

int playerStateCanHoldBall(Player * player)
{
	if (player->Health <= 0)
		return 0;

	if (player->PlayerState == PLAYER_STATE_DEATH
	 || player->PlayerState == PLAYER_STATE_DEATH_FALL
	 || player->PlayerState == PLAYER_STATE_LAVA_DEATH
	 || player->PlayerState == PLAYER_STATE_QUICKSAND_SINK
	 || player->PlayerState == PLAYER_STATE_DEATH_NO_FALL)
	 return 0;

	return 1;
}

void tryEquipBall(GridironPlayerState_t * playerState)
{
	if (!playerState || !playerState->Player || !GridironState.BallMoby)
		return;

	Player * player = playerState->Player;
	if (!playerStateCanHoldBall(player))
		return;

	if (player->HeldMoby)
		return;

	// hold
	ballPickup(GridironState.BallMoby, player);
	GridironState.BallCarrier = player;
}

void tryDropBall(GridironPlayerState_t * playerState)
{
	if (!playerState || !playerState->Player || !GridironState.BallMoby)
		return;

	// force player to drop flag
	Player * player = playerState->Player;
	if (player->HeldMoby == GridironState.BallMoby)
	{
		playerDropFlag(player, 0);
		playerState->TimeLastCarrier = gameGetTime();
	}

	// reset
	if (GridironState.BallCarrier == player)
		GridironState.BallCarrier = NULL;
}

void playerLogic(GridironPlayerState_t * playerState)
{
	Player * localPlayer = (Player*)0x00347AA0;
	VECTOR temp = {0,0,0,0};
	int gameTime = gameGetTime();

	// 
	if (GridironState.BallCarrier == NULL)
	{
		if ((gameTime - playerState->TimeLastCarrier) > BALL_CARRY_COOLDOWN)
		{
			vector_subtract(temp, GridironState.BallMoby->Position, playerState->Player->PlayerPosition);
			//if (vector_length(temp) < 2)
				tryEquipBall(playerState);
		}
	}
	else if (GridironState.BallCarrier == playerState->Player)
	{
		// get hero state
		int state = playerState->Player->PlayerState;

		// check if dropped
		if (playerState->Player->HeldMoby != GridironState.BallMoby)
		{
			playerState->TimeLastCarrier = gameGetTime();
			GridironState.BallCarrier = NULL;
		}
		
		else if (!playerStateCanHoldBall(playerState->Player))
			tryDropBall(playerState);

		else if (playerPadGetButton(playerState->Player, PAD_R1) && playerState->Player->WeaponHeldId != WEAPON_ID_SWINGSHOT)
		{
			if (state == PLAYER_STATE_CHARGE
			 || state == PLAYER_STATE_WALK
			 || state == PLAYER_STATE_IDLE
			 || state == PLAYER_STATE_THROW_SHURIKEN
			 || state == PLAYER_STATE_JUMP
			 || state == PLAYER_STATE_DOUBLE_JUMP
			 || state == PLAYER_STATE_SKID
			 || state == PLAYER_STATE_SLIDE
			 || state == PLAYER_STATE_SLOPESLIDE
			 )
			{
				PlayerVTable * table = playerGetVTable(playerState->Player);
				table->UpdateState(playerState->Player, PLAYER_STATE_THROW_SHURIKEN, 1, 1, 1);
			}
		}
		else if (playerPadGetButtonUp(playerState->Player, PAD_R1) && state == PLAYER_STATE_THROW_SHURIKEN)
		{
			tryDropBall(playerState);
			ballThrow(GridironState.BallMoby);
		}

	}
}

void loadGameplayHook(void * gameplayMobies, void * a1, u32 a2)
{
	DPRINTF("loading gameplay at %08X\n", (u32)gameplayMobies);

	//
	int mobyCount = *(int*)gameplayMobies;
	GameplayMobyDef_t* defs = (GameplayMobyDef_t*)((u32)gameplayMobies + 0x10);
	int i;
	int nodeCount = 0;
	VECTOR empty = {0,0,0,0};
	float * point;

	for (i = 0; i < mobyCount; ++i)
	{
		if (defs->OClass == MOBY_ID_NODE_BASE)
		{
			
			++nodeCount;
		}

		++defs;
	}

	// call load gameplay func
	((void (*)(void*,void*,u32))0x004ECF70)(gameplayMobies, a1, a2);
}

void findFlagBases(void)
{
	// check if we need to refind the bases
	if (GridironState.Base[0] && GridironState.Base[1])
		return;

	// reset
	GridironState.Base[0] = 0;
	GridironState.Base[1] = 0;
	int count = 0;

	Moby* moby = mobyGetFirst();
	while (moby)
	{
		if (moby->MobyId == 0x266E && moby->UNK_34[0] == 0x14)
		{
			void * pvars = moby->PropertiesPointer;
			int team = -1;
			if (pvars)
				team = *(char*)pvars;

			if (team >= 0 && team < 2 && count < 2)
			{
				DPRINTF("flag at %08X %08X %d\n", (u32)moby, (u32)pvars, team);
				GridironState.Base[team] = moby;
				++count;
			}
		}

		moby = moby->NextMoby;
	}
}

/*
 * NAME :		initialize
 * 
 * DESCRIPTION :
 * 			Initializes the game mode.
 * 
 * NOTES :
 * 			This is called once at start.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void initialize(void)
{
	int i = 0;
	GuberMoby * guberMoby = guberMobyGetFirst();
	GameOptions * gameOptions = gameGetOptions();
	Player ** players = playerGetAll();

	// Reset snd state
	GridironState.GameOver = 0;
	GridironState.Base[0] = 0;
	GridironState.Base[1] = 0;
	GridironState.BallMoby = 0;
	GridironState.BallCarrier = 0;

	// Remove blip type write
	//*(u32*)0x00553C5C = 0;

	// Overwrite 'you picked up a weapon pack' string to pickup bomb message
	//replaceString(0x2331, SND_BOMB_YOU_PICKED_UP);

	// Write patch to hook GuberMoby event handler
	*(u32*)0x0061CB30 = 0x8C460014; // move func ptr to a2
	*(u32*)0x0061CB38 = 0x0C000000 | ((u32)&GuberMobyEventHandler / 4); // call our func

	// spawn ball
	GridironState.BallMoby = ballSpawn(BallSpawnPoint);

	// disable flag pickup
	*(u32*)0x00418910 = 0;
	*(u32*)0x0041891C = 0;
	*(u32*)0x00417EB0 = 0;

	// find and hide flags
	Moby * moby = mobyGetFirst();
	while (moby)
	{
		switch (moby->MobyId)
		{
			case MOBY_ID_BLUE_FLAG:
			case MOBY_ID_RED_FLAG:
			case MOBY_ID_GREEN_FLAG:
			case MOBY_ID_ORANGE_FLAG:
			{
				// disables the rendering of
				moby->UNK_34[0] |= 1;
				break;
			}
		}

		moby = moby->NextMoby;
	}

	// Initialize players
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		GridironState.Players[i].Player = players[i];
		GridironState.Players[i].TimeLastCarrier = 0;
	}
	
	Initialized = 1;
	InitializedTime = gameGetTime();
}

/*
 * NAME :		gameStart
 * 
 * DESCRIPTION :
 * 			Snd game logic entrypoint.
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
	int i = 0;
	GameSettings * gameSettings = gameGetSettings();
	Player * localPlayer = (Player*)0x00347AA0;
	Player ** players = playerGetAll();
	int gameTime = gameGetTime();
	GameData * gameData = gameGetData();

	dlPreUpdate();

	// Ensure in game
	if (!gameSettings)
		return;

	// Determine if host
	GridironState.IsHost = gameIsHost(localPlayer->Guber.Id.GID.HostId);

	// Initialize if not yet initialized
	if (!Initialized)
		initialize();


#if DEBUG
	if (!GridironState.GameOver && padGetButton(0, PAD_L3 | PAD_R3) > 0)
		GridironState.GameOver = 1;
#endif

	if (!gameHasEnded() && gameIsIn() && !GridironState.GameOver)
	{
		// game is running
		findFlagBases();

		// process players
		for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		{
			GridironPlayerState_t * p = &GridironState.Players[i];
			if (p->Player)
				playerLogic(p);
		}
	}
	else
	{
		// game over
		if (GridironState.GameOver == 1)
		{
			gameEnd(4);
			GridironState.GameOver = 2;
		}
	}

	dlPostUpdate();
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

/*
 * NAME :		loadStart
 * 
 * DESCRIPTION :
 * 			SND load logic entrypoint.
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
	// Hook load gameplay file
	if (*(u32*)0x004EE664 == 0x0C13B3DC)
		*(u32*)0x004EE664 = 0x0C000000 | (u32)&loadGameplayHook / 4;
}
