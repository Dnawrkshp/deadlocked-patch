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

#include <libdl/time.h>
#include <libdl/game.h>
#include <libdl/gamesettings.h>
#include <libdl/player.h>
#include <libdl/weapon.h>
#include <libdl/hud.h>
#include <libdl/cheats.h>
#include <libdl/sha1.h>
#include <libdl/dialog.h>
#include <libdl/ui.h>
#include <libdl/stdio.h>
#include <libdl/graphics.h>
#include <libdl/net.h>
#include "module.h"
#include "messageid.h"


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

#define SPLEEF_BOARD_DIMENSION							(10)
#define SPLEEF_BOARD_LEVELS									(2)
#define SPLEEF_BOARD_LEVEL_OFFSET						(40.0)
#define SPLEEF_BOARD_BOX_SIZE								(4.0)
#define SPLEEF_BOARD_SPAWN_RADIUS						(SPLEEF_BOARD_BOX_SIZE * ((SPLEEF_BOARD_DIMENSION + SPLEEF_BOARD_DIMENSION) / 5))
#define SPLEEF_BOARD_BOX_MAX								(SPLEEF_BOARD_DIMENSION * SPLEEF_BOARD_DIMENSION * SPLEEF_BOARD_LEVELS)


const char * SPLEEF_ROUND_WIN = "First!";
const char * SPLEEF_ROUND_SECOND = "Second!";
const char * SPLEEF_ROUND_THIRD = "Third!";
const char * SPLEEF_ROUND_LOSS = "Better luck next time!";

/*
 *
 */
int Initialized = 0;

struct SpleefState
{
	int RoundNumber;
	int RoundStartTicks;
	int RoundEndTicks;
	char RoundResult[4];
	char RoundPlayerState[GAME_MAX_PLAYERS];
	int RoundInitialized;
	int GameOver;
	int WinningTeam;
	int IsHost;
} SpleefState;

/*
 *
 */
typedef struct SpleefOutcomeMessage
{
	char Outcome[4];
} SpleefOutcomeMessage_t;

/*
 *
 */
typedef struct SpleefDestroyBoxMessage
{
	int BoxId;
} SpleefDestroyBoxMessage_t;

/*
 *
 */
ScoreboardItem PlayerScores[GAME_MAX_PLAYERS];

/*
 *
 */
ScoreboardItem * SortedPlayerScores[GAME_MAX_PLAYERS];

/*
 *
 */
Moby* SpleefBox[SPLEEF_BOARD_DIMENSION * SPLEEF_BOARD_DIMENSION * SPLEEF_BOARD_LEVELS];

/*
 *
 */
Moby* SourceBox;

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


// forwards
void onSetRoundOutcome(char outcome[4]);
int onSetRoundOutcomeRemote(void * connection, void * data);
void setRoundOutcome(int first, int second, int third);

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
void sortScoreboard(void)
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
				(SortedPlayerScores[i]->UNK != 1 &&
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

void getWinningPlayer(int * winningPlayerId, int * winningPlayerScore)
{
	int i;
	int pId = 0;
	int pScore = 0;
	Player ** players = playerGetAll();
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		if (PlayerScores[i].Value > pScore)
		{
			pId = i;
			pScore = PlayerScores[i].Value;
		}
	}

	*winningPlayerScore = pScore;
	*winningPlayerId = pId;
}

void onSetRoundOutcome(char outcome[4])
{
	memcpy(SpleefState.RoundResult, outcome, 4);
	DPRINTF("outcome set to %d,%d,%d,%d\n", outcome[0], outcome[1], outcome[2], outcome[3]);
}

int onSetRoundOutcomeRemote(void * connection, void * data)
{
	SpleefOutcomeMessage_t * message = (SpleefOutcomeMessage_t*)data;
	onSetRoundOutcome(message->Outcome);

	return sizeof(SpleefOutcomeMessage_t);
}

void setRoundOutcome(int first, int second, int third)
{
	SpleefOutcomeMessage_t message;

	// don't allow overwriting existing outcome
	if (SpleefState.RoundResult[0])
		return;

	// don't allow changing outcome when not host
	if (!SpleefState.IsHost)
		return;

	// send out
	message.Outcome[0] = 1;
	message.Outcome[1] = first;
	message.Outcome[2] = second;
	message.Outcome[3] = third;
	netBroadcastCustomAppMessage(netGetDmeServerConnection(), CUSTOM_MSG_ID_SEARCH_AND_DESTROY_SET_OUTCOME, sizeof(SpleefOutcomeMessage_t), &message);

	// set locally
	onSetRoundOutcome(message.Outcome);
}

void onDestroyBox(int id)
{
	Moby* box = SpleefBox[id];
	if (box && box->MobyId == MOBY_ID_NODE_BOLT_GUARD && box->NextMoby)
	{
		mobyDestroy(box);
	}

	DPRINTF("box destroyed %d\n", id);
}

int onDestroyBoxRemote(void * connection, void * data)
{
	SpleefDestroyBoxMessage_t * message = (SpleefDestroyBoxMessage_t*)data;

	// if the round hasn't ended
	if (!SpleefState.RoundEndTicks)
		onDestroyBox(message->BoxId);

	return sizeof(SpleefDestroyBoxMessage_t);
}

void destroyBox(int id)
{
	SpleefDestroyBoxMessage_t message;

	// send out
	message.BoxId = id;
	netBroadcastCustomAppMessage(netGetDmeServerConnection(), CUSTOM_MSG_ID_SPLEEF_DESTROY_BOX, sizeof(SpleefDestroyBoxMessage_t), &message);
	DPRINTF("sent destroy box %d\n", id);
}

void changeBoxCollisionIds(void * modelPtr)
{
	int i = 0;

	// Collision offset at +0x10
	u32 colPtr = *(u32*)((u32)modelPtr + 0x10) + 0x70;

	for (i = 0; i < 12; ++i)
	{
		*(u8*)(colPtr + 3) = 0x02;
		colPtr += 4;
	}
}

void boxUpdate(Moby* moby)
{
	int i;
	MobyColDamage* colDamage = mobyGetDamage(moby, 0xfffffff, 0);

	if (moby->UNK_2C[0] != -1 && colDamage && colDamage->Moby == moby)
	{
		int damagePlayerId = colDamage->Damager->NetObjectGid.HostId;
		if (playerIdIsLocal(damagePlayerId))
		{
			// call base
			((void (*)(Moby*))0x00427450)(moby);

			for (i = 0; i < SPLEEF_BOARD_BOX_MAX; ++i)
			{
				if (SpleefBox[i] == moby)
				{
					destroyBox(i);
					return;
				}
			}
		}

		// remove damage
		moby->UNK_2C[0] = 0xff;
	}

	// call base
	((void (*)(Moby*))0x00427450)(moby);
}

void drawRoundMessage(const char * message, float scale)
{
	GameSettings * gameSettings = gameGetSettings();
	char* rankStrings[] = { "1st", "2nd", "3rd" };
	int fw = gfxGetFontWidth(message, -1, scale);
	float x = 0.5;
	float y = 0.16;
	float p = 0.02;
	float w = maxf(196.0, fw);
	float h = 120.0;
	int i;

	// draw container
	gfxScreenSpaceBox(x-(w/(SCREEN_WIDTH*2.0)), y, (w / SCREEN_WIDTH) + p, (h / SCREEN_HEIGHT) + p, 0x20ffffff);

	// draw message
	y *= SCREEN_HEIGHT;
	x *= SCREEN_WIDTH;
	gfxScreenSpaceText(x, y + 5, scale, scale * 1.5, 0x80FFFFFF, message, -1, 1);

	// draw ranks
	y += 24.0 * scale;
	scale *= 0.5;
	w /= 1.5;
	for (i = 1; i < 4; ++i)
	{
		int pId = SpleefState.RoundResult[i];
		if (pId >= 0)
		{
			y += 18.0 * scale;
			gfxScreenSpaceText(x-(w/2), y, scale, scale, 0x80FFFFFF, rankStrings[i-1], -1, 3);
			gfxScreenSpaceText(x+(w/2), y, scale, scale, 0x80FFFFFF, gameSettings->PlayerNames[pId], -1, 5);
		}
	}
}

int playerIsDead(Player* p)
{
	return p->PlayerState == 57 // dead
								|| p->PlayerState == 106 // drown
								|| p->PlayerState == 118 // death fall
								|| p->PlayerState == 122 // death sink
								|| p->PlayerState == 123 // death lava
								|| p->PlayerState == 148 // death no fall
								;
}

void resetRoundState(void)
{
	GameSettings * gameSettings = gameGetSettings();
	Player ** players = playerGetAll();
	int gameTime = gameGetTime();
	int i,j,k, count=0;
	VECTOR pos, rot = {0,0,0,0}, center;
	Moby* hbMoby = 0;

	// 
	SpleefState.RoundInitialized = 0;
	SpleefState.RoundStartTicks = gameTime;
	SpleefState.RoundEndTicks = 0;
	SpleefState.RoundResult[0] = 0;
	SpleefState.RoundResult[1] = -1;
	SpleefState.RoundResult[2] = -1;
	SpleefState.RoundResult[3] = -1;
	memset(SpleefState.RoundPlayerState, -1, GAME_MAX_PLAYERS);

	// Center
	center[0] = StartPos[0] + (SPLEEF_BOARD_BOX_SIZE * (SPLEEF_BOARD_DIMENSION / (float)2.0));
	center[1] = StartPos[1] + (SPLEEF_BOARD_BOX_SIZE * (SPLEEF_BOARD_DIMENSION / (float)2.0));
	center[2] = StartPos[2];

	// spawn players
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		if (!p)
			continue;

		// set state
		SpleefState.RoundPlayerState[i] = 0;

		// move to spawn
		float theta = (p->PlayerId / (float)gameSettings->PlayerCount) * (float)2.0 * MATH_PI;
		while (theta > MATH_TAU)
			theta -= MATH_PI;

		pos[0] = center[0] + (cosf(theta) * SPLEEF_BOARD_SPAWN_RADIUS);
		pos[1] = center[1] + (sinf(theta) * SPLEEF_BOARD_SPAWN_RADIUS);
		pos[2] = center[2] + (float)30;

		// 
		rot[2] = theta - MATH_PI;

		// 
		playerRespawn(p);
		playerSetPosRot(p, pos, rot);
	}

	// reset boxes
	vector_copy(pos, StartPos);
	memset(rot, 0, sizeof(rot));
	pos[3] = SourceBox->Position[3];

	// Spawn boxes
	for (k = 0; k < SPLEEF_BOARD_LEVELS; ++k)
	{
		for (i = 0; i < SPLEEF_BOARD_DIMENSION; ++i)
		{
			for (j = 0; j < SPLEEF_BOARD_DIMENSION; ++j)
			{
				// delete old one
				int boxId = (k * SPLEEF_BOARD_DIMENSION * SPLEEF_BOARD_DIMENSION) + (i * SPLEEF_BOARD_DIMENSION) + j;
				if (!SpleefBox[boxId] || SpleefBox[boxId]->MobyId != MOBY_ID_NODE_BOLT_GUARD || !SpleefBox[boxId]->NextMoby)
				{
					// spawn
					SpleefBox[boxId] = hbMoby = mobySpawn(MOBY_ID_NODE_BOLT_GUARD, 0);

					if (hbMoby)
					{
						vector_copy(hbMoby->Position, pos);

						hbMoby->UNK_30 = 0xFF;
						hbMoby->UNK_31 = 0x01;
						hbMoby->RenderDistance = 0x0080;
						hbMoby->Opacity = 0x80;
						hbMoby->UNK_20[0] = 1;

						hbMoby->Scale = (float)0.0425 * SPLEEF_BOARD_BOX_SIZE;
						hbMoby->UNK_38[0] = 2;
						hbMoby->UNK_38[1] = 2;
						hbMoby->GuberMoby = 0;
						hbMoby->UNK_A8 = &boxUpdate;

						// For this model the vector here is copied to 0x80 in the moby
						// This fixes the occlusion bug
						hbMoby->AnimationPointer = StartUNK_80;

						// Copy from source box
						hbMoby->ModelPointer = SourceBox->ModelPointer;
						hbMoby->CollisionPointer = SourceBox->CollisionPointer;
						hbMoby->UNK_20[2] = SourceBox->UNK_20[2];

						++count;
					}
				}

				pos[1] += SPLEEF_BOARD_BOX_SIZE;
			}

			pos[0] += SPLEEF_BOARD_BOX_SIZE;
			pos[1] = StartPos[1];
		}

		pos[0] = StartPos[0];
		pos[1] = StartPos[1];
		pos[2] -= SPLEEF_BOARD_LEVEL_OFFSET;
	}

	// 
	SpleefState.RoundInitialized = 1;

	// this has to be here otherwise the rounds won't reset correctly
	// think that this eats cycles and that helps sync things maybe?
	// not super sure
	printf("count: %d, source: %08x, new: %08x\n", count, (u32)SourceBox, (u32)hbMoby);

	// 
	#if DEBUG
		if (hbMoby)
			hbMoby->Opacity = 0xFF;
		printf("Round %d started\n", SpleefState.RoundNumber);
	#endif
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
	GameSettings * gameSettings = gameGetSettings();
	GameOptions * gameOptions = gameGetOptions();
	Player ** players = playerGetAll();
	int i;

	// Set death barrier
	gameSetDeathHeight(StartPos[2] - (SPLEEF_BOARD_LEVEL_OFFSET * SPLEEF_BOARD_LEVELS) - 10);

	// Set survivor
	gameOptions->GameFlags.MultiplayerGameFlags.Survivor = 1;
	gameOptions->GameFlags.MultiplayerGameFlags.RespawnTime = -1;
	gameOptions->GameFlags.MultiplayerGameFlags.Teamplay = 0;

	// Hook set outcome net event
	netInstallCustomMsgHandler(CUSTOM_MSG_ID_SEARCH_AND_DESTROY_SET_OUTCOME, &onSetRoundOutcomeRemote);
	netInstallCustomMsgHandler(CUSTOM_MSG_ID_SPLEEF_DESTROY_BOX, &onDestroyBoxRemote);
	
	// Disable normal game ending
	*(u32*)0x006219B8 = 0;	// survivor (8)
	*(u32*)0x00621A10 = 0;  // survivor (8)

	// Spawn box so we know the correct model and collision pointers
	SourceBox = mobySpawn(MOBY_ID_BETA_BOX, 0);
	
	// change collision ids
	changeBoxCollisionIds(SourceBox->ModelPointer);

	// clear spleefbox array
	memset(SpleefBox, 0, sizeof(SpleefBox));

	// Initialize scoreboard
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		PlayerScores[i].TeamId = p ? i : 0;
		PlayerScores[i].UNK = playerIsLocal(p);
		PlayerScores[i].Value = 0;
		SortedPlayerScores[i] = &PlayerScores[i];
	}

	//
	sortScoreboard();
	updateScoreboard();

	// initialize state
	SpleefState.GameOver = 0;
	SpleefState.RoundNumber = 0;
	resetRoundState();

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
	Player * localPlayer = (Player*)0x00347AA0;
	int i;

	// Ensure in game
	if (!gameSettings || !gameIsIn())
		return;

	// Determine if host
	SpleefState.IsHost = gameIsHost(localPlayer->Guber.Id.GID.HostId);

	if (!Initialized)
		initialize();

	int killsToWin = gameGetOptions()->GameFlags.MultiplayerGameFlags.KillsToWin;

#if DEBUG
	if (padGetButton(0, PAD_L3 | PAD_R3) > 0)
		SpleefState.GameOver = 1;
#endif

	if (!gameHasEnded() && !SpleefState.GameOver)
	{
		if (SpleefState.RoundResult[0])
		{
			if (SpleefState.RoundEndTicks)
			{
				// draw round message
				if (SpleefState.RoundResult[1] == localPlayer->PlayerId)
				{
					drawRoundMessage(SPLEEF_ROUND_WIN, 1.5);
				}
				else if (SpleefState.RoundResult[2] == localPlayer->PlayerId)
				{
					drawRoundMessage(SPLEEF_ROUND_SECOND, 1.5);
				}
				else if (SpleefState.RoundResult[3] == localPlayer->PlayerId)
				{
					drawRoundMessage(SPLEEF_ROUND_THIRD, 1.5);
				}
				else
				{
					drawRoundMessage(SPLEEF_ROUND_LOSS, 1.5);
				}

				// handle when round properly ends
				if (gameGetTime() > SpleefState.RoundEndTicks)
				{
					// increment round
					++SpleefState.RoundNumber;

					// reset round state
					resetRoundState();
				}
			}
			else
			{
				// Handle game outcome
				for (i = 1; i < 4; ++i)
				{
					int playerIndex = SpleefState.RoundResult[i];
					if (playerIndex >= 0) {
						PlayerScores[playerIndex].Value += 4 - i;
						DPRINTF("player %d score %d\n", playerIndex, PlayerScores[playerIndex].Value);
					}
				}

				// update scoreboard
				sortScoreboard();
				GAME_SCOREBOARD_REFRESH_FLAG = 1;

				// set when next round starts
				SpleefState.RoundEndTicks = gameGetTime() + (TIME_SECOND * 5);

				// update winner
				int winningScore = 0;
				getWinningPlayer(&SpleefState.WinningTeam, &winningScore);
				if (killsToWin > 0 && winningScore >= killsToWin)
				{
					SpleefState.GameOver = 1;
				}
			}
		}
		else
		{
			// host specific logic
			if (SpleefState.IsHost && (gameGetTime() - SpleefState.RoundStartTicks) > (5 * TIME_SECOND))
			{
				int playersAlive = 0, playerCount = 0, lastPlayerAlive = -1;
				for (i = 0; i < GAME_MAX_PLAYERS; ++i)
				{
					if (SpleefState.RoundPlayerState[i] >= 0)
						++playerCount;
					if (SpleefState.RoundPlayerState[i] == 0)
						++playersAlive;
				}

				for (i = 0; i < GAME_MAX_PLAYERS; ++i)
				{
					Player * p = players[i];

					if (p)
					{
						// check if player is dead
						if (playerIsDead(p) || SpleefState.RoundPlayerState[i] == 1)
						{
							// player newly died
							if (SpleefState.RoundPlayerState[i] == 0)
							{
								DPRINTF("player %d died\n", i);
								SpleefState.RoundPlayerState[i] = 1;

								// set player to first/second/third if appropriate
								if (playersAlive < 4)
								{
									SpleefState.RoundResult[playersAlive] = i;
									DPRINTF("setting %d place to player %d\n", playersAlive, i);
								}
							}
						}
						else
						{
							lastPlayerAlive = i;
						}
					}
				}

				if ((playersAlive == 1 && playerCount > 1) || playersAlive == 0)
				{
					// end
					DPRINTF("end round: playersAlive:%d playerCount:%d\n", playersAlive, playerCount);
					if (lastPlayerAlive >= 0)
					{
						SpleefState.RoundResult[1] = lastPlayerAlive;
						DPRINTF("last player alive is %d\n", lastPlayerAlive);
					}
					setRoundOutcome(SpleefState.RoundResult[1], SpleefState.RoundResult[2], SpleefState.RoundResult[3]);
				}
			}
		}
	}
	else
	{
		// set winner
		gameSetWinner(SpleefState.WinningTeam, 1);

		// end game
		if (SpleefState.GameOver == 1)
		{
			gameEnd(4);
			SpleefState.GameOver = 2;
		}
	}

	// 
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		if (!p)
			continue;

		if (!playerIsDead(p))
			p->Health = PLAYER_MAX_HEALTH;
		else
			p->Health = 0;
	}

	// 
	updateScoreboard();

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
