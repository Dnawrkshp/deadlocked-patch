/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Duck hunt entrypoint and logic.
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

#define HUNT_MAX_HUNTER_COUNT								(10)
#define HUNT_ROUND_TIMELIMIT								(120)

const char * HUNT_ROUND_WIN = "First!";
const char * HUNT_ROUND_SECOND = "Second!";
const char * HUNT_ROUND_THIRD = "Third!";
const char * HUNT_ROUND_OVER = "Round over!";
const char * HUNT_HUNTER_HELLO = "Shoot the ducks!";
const char * HUNT_DUCK_HELLO = "Race to the finish!";

/*
 *
 */
int Initialized = 0;

enum HuntPlayerState
{
	PLAYER_STATE_INVALID = -1,
	PLAYER_STATE_DUCK_ALIVE,
	PLAYER_STATE_DUCK_DEAD,
	PLAYER_STATE_DUCK_FINISHED,
	PLAYER_STATE_HUNTER
};

struct HuntState
{
	int RoundNumber;
	int RoundStartTicks;
	int RoundEndTicks;
	char RoundOver;
	char RoundResult[GAME_MAX_PLAYERS];
	char RoundPlayerState[GAME_MAX_PLAYERS];
	char PlayerHunterCounter[GAME_MAX_PLAYERS];
	short PlayerScore[GAME_MAX_PLAYERS];
	int RoundPlayerTime[GAME_MAX_PLAYERS];
	int PlayerBestTime[GAME_MAX_PLAYERS];
	int RoundInitialized;
	int GameOver;
	int WinningTeam;
	int IsHost;
	u32 Seed;
} HuntState;

/*
 *
 */
typedef struct HuntOutcomeMessage
{
	// gives the rank ordering for each player
	// Outcomes[0] is first place where the value is the player id
	// values of -1 are considered empty
	char Outcomes[GAME_MAX_PLAYERS];

	// how quickly each player took to finish
	// 0 is DNF
	int PlayerTimes[GAME_MAX_PLAYERS];
} HuntOutcomeMessage_t;

/*
 *
 */
ScoreboardItem PlayerScores[GAME_MAX_PLAYERS];

/*
 *
 */
ScoreboardItem * SortedPlayerScores[GAME_MAX_PLAYERS];

/*
 * Position that boxes are spawned to.
 */
VECTOR HunterStartPos = {
	350,
	130,
	109,
	0
};

/*
 * Position where the ducks will finish.
 * The 4th float represents the radius of the sphere used to check
 * if a duck has finished.
 */
VECTOR DuckFinishPos = {
	500,
	272,
	111,
	10
};

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
				 (SortedPlayerScores[i+1]->UNK == 1 && !dontLockLocal))))
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
	PlayerGameStats * stats = gameGetPlayerStats();

	// Update player scores
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		PlayerScores[i].Value = stats->Kills[i];
	}

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

void onSetRoundOutcome(HuntOutcomeMessage_t * message)
{
	int i;

	// copy outcome
	memcpy(HuntState.RoundResult, message->Outcomes, sizeof(message->Outcomes));

	// copy times
	memcpy(HuntState.RoundPlayerTime, message->PlayerTimes, sizeof(message->PlayerTimes));

	// save best times
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		if (HuntState.RoundPlayerTime[i] < HuntState.PlayerBestTime[i])
			HuntState.PlayerBestTime[i] = HuntState.RoundPlayerTime[i];

	// set round over
	HuntState.RoundOver = 1;

#if DEBUG
	printf("outcome received: \n");
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		int pid = message->Outcomes[i];
		if (pid >= 0)
			printf("\tRank %d: PID:%d Time:%f\n", i+1, pid, message->PlayerTimes[pid] / (float)TIME_SECOND);
		else
			printf("\tRank %d: PID:%d Time:DNF\n", i+1, pid);
	}
#endif
}

int onSetRoundOutcomeRemote(void * connection, void * data)
{
	HuntOutcomeMessage_t message;

	memcpy(&message, data, sizeof(HuntOutcomeMessage_t));
	onSetRoundOutcome(&message);

	return sizeof(HuntOutcomeMessage_t);
}

void setRoundOutcome()
{
	int i, j;
	HuntOutcomeMessage_t message;
	char pidUsed[GAME_MAX_PLAYERS];

	// don't allow overwriting existing outcome
	if (HuntState.RoundOver)
		return;

	// don't allow changing outcome when not host
	if (!HuntState.IsHost)
		return;

	// copy times
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		message.PlayerTimes[i] = HuntState.RoundPlayerTime[i];

	// determine rank ordering based off times
	memset(pidUsed, 0, sizeof(pidUsed));
	for (j = 0; j < GAME_MAX_PLAYERS; ++j)
	{
		int pid = -1;
		int time = -1;
		for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		{
			int pState = HuntState.RoundPlayerState[i];
			if (!pidUsed[i] && pState != PLAYER_STATE_INVALID && pState != PLAYER_STATE_HUNTER && message.PlayerTimes[i] > time)
			{
				time = message.PlayerTimes[i];
				pid = i;
			}
		}

		if (pid >= 0)
			pidUsed[pid] = 1;
		message.Outcomes[j] = pid;
	}

	// send out
	netBroadcastCustomAppMessage(netGetDmeServerConnection(), CUSTOM_MSG_ID_SEARCH_AND_DESTROY_SET_OUTCOME, sizeof(HuntOutcomeMessage_t), &message);

	// set locally
	onSetRoundOutcome(&message);
}

void drawRoundMessage(const char * message, float scale)
{
	GameSettings * gameSettings = gameGetSettings();
	char buf[128];
	int fw = gfxGetFontWidth(message, -1, scale);
	float x = 0.5;
	float y = 0.16;
	float p = 0.02;
	float w = maxf(300.0, fw);
	float h = 39.0;
	int i;

	// grow container by number of players
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		if (HuntState.RoundPlayerState[i] != PLAYER_STATE_INVALID)
			h += 27.0;

	// draw container
	gfxScreenSpaceBox(x-(w/(SCREEN_WIDTH*2.0)), y, (w / SCREEN_WIDTH) + p, (h / SCREEN_HEIGHT) + p, 0x20ffffff);

	// draw message
	y *= SCREEN_HEIGHT;
	x *= SCREEN_WIDTH;
	gfxScreenSpaceText(x, y + 5, scale, scale * 1.5, 0x80FFFFFF, message, -1, 1);

	// draw ranks
	y += 24.0 * scale;
	scale *= 0.5;
	w /= 1.08;
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		int pId = HuntState.RoundResult[i];
		if (pId >= 0)
		{
			int pTime = HuntState.RoundPlayerTime[pId];
			y += 18.0 * scale;

			// draw rank only if finished
			if (pTime > 0)
			{
				sprintf(buf, "%d", i+1);
				gfxScreenSpaceText(x-(w/2), y, scale, scale, 0x80FFFFFF, buf, -1, 3);
			}

			// draw name
			gfxScreenSpaceText(x, y, scale, scale, 0x80FFFFFF, gameSettings->PlayerNames[pId], -1, 4);

			// draw time
			if (pTime == 0)
			{
				// did not finish
				gfxScreenSpaceText(x+(w/2), y, scale, scale, 0x801515E0, "DNF", -1, 5);
			}
			else
			{
				// time as minutes:seconds
				sprintf(buf, "%02d:%02d", pTime / TIME_MINUTE, (pTime % TIME_MINUTE) / TIME_SECOND);
				gfxScreenSpaceText(x+(w/2), y, scale, scale, 0x80FFFFFF, buf, -1, 5);
			}
		}
	}
}

void onPlayerFinished(int playerId)
{
	// round already ended
	if (HuntState.RoundOver)
		return;
	
	// only let host set player finish
	if (!HuntState.IsHost)
		return;

	// only a live duck can finish
	if (HuntState.RoundPlayerState[playerId] != PLAYER_STATE_DUCK_ALIVE)
		return;

	// set time
	HuntState.RoundPlayerTime[playerId] = gameGetTime() - HuntState.RoundStartTicks;
	HuntState.RoundPlayerState[playerId] = PLAYER_STATE_DUCK_FINISHED;
	DPRINTF("player %d finished\n", playerId);
}

int getRandomPotentialHunter(u32 seed)
{
	Player ** playerObjects = playerGetAll();

	int value = (seed % GAME_MAX_PLAYERS) + 1;
	int i = 0;
	int counter = 0;

	while (counter < value)
	{
		for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		{
			if (playerObjects[i] && HuntState.PlayerHunterCounter[i] < HUNT_MAX_HUNTER_COUNT)
			{
				++counter;

				if (value == counter)
					return i;
			}
		}

		// This means that there are none left
		if (counter == 0)
			return -1;
	}

	return -1;
}

int getPlayerIdFromMoby(Moby * moby)
{
	int i;
	Player ** players = playerGetAll();
	if (!moby)
		return -1;

	// find root parent
	while (moby->ParentMoby)
		moby = moby->ParentMoby;

	// check if guber is player
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		if ((u32)players[i] == (u32)moby->GuberMoby)
			return i;
	}
	
	return -1;
}

void collDamageMobyDirectHook(MobyColDamage * CollDamage, void * a1, int id, void * a3, MobyColDamageIn * CollDamageIn, Moby * moby)
{
	int damagerPlayerId = getPlayerIdFromMoby(CollDamageIn->Damager);
	int hitPlayerId = getPlayerIdFromMoby(moby);
	DPRINTF("CollDamageMobyDirect 0x%08X(%d) SRCMOBY: 0x%08X(%04X) DESTMOBY: 0x%08X(%04X) DPID:%d VPID:%d\n",
		(u32)CollDamage, id,
		(u32)CollDamageIn->Damager, CollDamageIn->Damager->MobyId, 
		(u32)moby, moby->MobyId, 
		damagerPlayerId, hitPlayerId);

	// if damage is from player to another player
	// stop damage if the damager is not a hunter or if the hit player is not a living duck
	if (damagerPlayerId >= 0 && hitPlayerId >= 0 && 
		(HuntState.RoundPlayerState[damagerPlayerId] != PLAYER_STATE_HUNTER || HuntState.RoundPlayerState[hitPlayerId] != PLAYER_STATE_DUCK_ALIVE))
	{
		CollDamage->DamageHp = 0;
		CollDamage->DamageFlags = 0;
		CollDamage->Flags = 0;
		DPRINTF("ignoring damage\n");
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
	PlayerWeaponData * pWeapon = NULL;
	int gameTime = gameGetTime();
	int i = 0;
	VECTOR pos = {0,0,0,0}, rot = {0,0,0,0};

	// 
	HuntState.RoundInitialized = 0;
	HuntState.RoundStartTicks = gameTime;
	HuntState.RoundEndTicks = 0;
	HuntState.RoundOver = 0;
	memset(HuntState.RoundResult, -1, sizeof(HuntState.RoundResult));
	memset(HuntState.RoundPlayerTime, 0, sizeof(HuntState.RoundPlayerTime));
	memset(HuntState.RoundPlayerState, PLAYER_STATE_INVALID, GAME_MAX_PLAYERS);

	// Use SHA1 hash to create psuedo random seed based off last seed
	sha1((void*)&HuntState.Seed, 4, (void*)&HuntState.Seed, 4);

	// determine number of hunters
	int hunterCount = 1;

	// determine hunters randomly
	for (i = 0; i < hunterCount; ++i)
	{
		int newHunterId = getRandomPotentialHunter(HuntState.Seed);
		if (newHunterId >= 0)
		{
			HuntState.RoundPlayerState[newHunterId] = PLAYER_STATE_HUNTER;
			HuntState.PlayerHunterCounter[newHunterId] += 1;
		}
	}

	// spawn players
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		Player * p = players[i];
		if (!p)
			continue;

		// respawn
		playerRespawn(p);

		// move to hunter location
		if (HuntState.RoundPlayerState[i] == PLAYER_STATE_HUNTER)
		{
			float theta = (p->PlayerId / (float)gameSettings->PlayerCount) * (float)2.0 * MATH_PI;
			while (theta > MATH_TAU)
				theta -= MATH_PI;

			pos[0] = HunterStartPos[0] + (cosf(theta) * 2.5);
			pos[1] = HunterStartPos[1] + (sinf(theta) * 2.5);
			pos[2] = HunterStartPos[2];

			// 
			rot[2] = HunterStartPos[3];

			// 
			playerSetPosRot(p, pos, rot);

			// prevent player from getting hurt from sniper
			p->PlayerMoby->UNK_34[1] &= ~0x40;

			if (playerIsLocal(p))
			{
				// enable hud
				PlayerHUDFlags * hudFlags = hudGetPlayerFlags(p->LocalPlayerIndex);
				if (hudFlags)
				{
					hudFlags->Flags.Weapons = 1;
				}
			}
		}
		else
		{
			// set state to duck if not hunter
			HuntState.RoundPlayerState[i] = PLAYER_STATE_DUCK_ALIVE;

			// allow player to get hurt from sniper
			p->PlayerMoby->UNK_34[1] |= 0x40;

			// remove weapons
			pWeapon = playerGetWeaponData(i);
			if (pWeapon)
			{
				pWeapon[WEAPON_ID_WRENCH].Level = -1;
				pWeapon[WEAPON_ID_VIPERS].Level = -1;
				pWeapon[WEAPON_ID_MAGMA_CANNON].Level = -1;
				pWeapon[WEAPON_ID_ARBITER].Level = -1;
				pWeapon[WEAPON_ID_B6].Level = -1;
				pWeapon[WEAPON_ID_FUSION_RIFLE].Level = -1;
				pWeapon[WEAPON_ID_MINE_LAUNCHER].Level = -1;
				pWeapon[WEAPON_ID_FLAIL].Level = -1;
				pWeapon[WEAPON_ID_OMNI_SHIELD].Level = -1;
			}
			
			if (playerIsLocal(p))
			{
				// remove weapons from equipment slots
				playerSetLocalEquipslot(p->LocalPlayerIndex, 0, WEAPON_ID_EMPTY);
				playerSetLocalEquipslot(p->LocalPlayerIndex, 1, WEAPON_ID_EMPTY);
				playerSetLocalEquipslot(p->LocalPlayerIndex, 2, WEAPON_ID_EMPTY);
				playerSetWeapon(p, WEAPON_ID_WRENCH);

				// disable hud
				PlayerHUDFlags * hudFlags = hudGetPlayerFlags(p->LocalPlayerIndex);
				if (hudFlags)
				{
					hudFlags->Flags.Weapons = 0;
				}
			}
		}
	}

	// 
	HuntState.RoundInitialized = 1;
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

	// Set survivor
	gameOptions->GameFlags.MultiplayerGameFlags.Survivor = 1;
	gameOptions->GameFlags.MultiplayerGameFlags.RespawnTime = -1;
	gameOptions->GameFlags.MultiplayerGameFlags.Teamplay = 0;

	// Hook set outcome net event
	netInstallCustomMsgHandler(CUSTOM_MSG_ID_SEARCH_AND_DESTROY_SET_OUTCOME, &onSetRoundOutcomeRemote);
	
	// Disable normal game ending
	*(u32*)0x006219B8 = 0;	// survivor (8)
	*(u32*)0x00621A10 = 0;  // survivor (8)
	*(u32*)0x00620F54 = 0;	// time end (1)

	// Disable wrench damage
	*(u32*)0x00439410 = 0;
	*(u32*)0x00503734 = 0;
	*(u32*)0x00439BAC = 0;
	*(u32*)0x00439BDC = 0;

	// Patch fusion rifle shot
	// *(u32*)0x003FC66C = 0x0C000000 | ((u32)&fusionRifleCollLineFixHook / 4);

	// disable packs
	cheatsApplyNoPacks();

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

	// hook damage
	*(u32*)0x005034f8 = 0x08000000 | ((u32)&collDamageMobyDirectHook / 4);

	// initialize state
	HuntState.GameOver = 0;
	HuntState.RoundNumber = 0;
	HuntState.Seed = gameSettings->SpawnSeed;
	memset(&HuntState.PlayerHunterCounter, 0, sizeof(HuntState.PlayerHunterCounter));
	memset(&HuntState.PlayerScore, 0, sizeof(HuntState.PlayerScore));
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
	GameData * gameData = gameGetData();
	int gameTime = gameGetTime();
	PlayerGameStats * stats = gameGetPlayerStats();
	int i;
	VECTOR vTemp;
	char buf[128];

	// Ensure in game
	if (!gameSettings || !gameIsIn())
		return;

	// Determine if host
	HuntState.IsHost = gameIsHost(localPlayer->Guber.Id.GID.HostId);

	if (!Initialized)
		initialize();

	int killsToWin = gameGetOptions()->GameFlags.MultiplayerGameFlags.KillsToWin;

#if DEBUG
	if (padGetButton(0, PAD_L3 | PAD_R3) > 0)
		HuntState.GameOver = 1;
#endif

	if (!gameHasEnded() && !HuntState.GameOver)
	{
		if (HuntState.RoundOver)
		{
			if (HuntState.RoundEndTicks)
			{
				// draw round message
				sprintf(buf, "%s  (%d/%d)", HUNT_ROUND_OVER, HuntState.RoundNumber+1, HUNT_MAX_HUNTER_COUNT * gameSettings->PlayerCount);
				drawRoundMessage(buf, 1.5);

				// handle when round properly ends
				if (gameGetTime() > HuntState.RoundEndTicks)
				{
					// increment round
					++HuntState.RoundNumber;

					// reset round state
					resetRoundState();
				}
			}
			else
			{
				// increase score of top 3 finishers
				for (i = 0; i < 3; ++i)
				{
					int playerIndex = HuntState.RoundResult[i];
					if (playerIndex >= 0 && HuntState.RoundPlayerState[playerIndex] == PLAYER_STATE_DUCK_FINISHED)
					{
						HuntState.PlayerScore[playerIndex] += 3 - i;
						stats->Kills[playerIndex] += 3 - i;
						DPRINTF("player %d score %d\n", playerIndex, stats->Kills[playerIndex]);
					}
				}

				// update scoreboard
				updateScoreboard();
				sortScoreboard(0);
				GAME_SCOREBOARD_REFRESH_FLAG = 1;

				// set when next round starts
				HuntState.RoundEndTicks = gameGetTime() + (TIME_SECOND * 5);

				// update winner
				int highestScore = 0;
				getWinningPlayer(&HuntState.WinningTeam, &highestScore);
				for (i = 0; i < GAME_MAX_PLAYERS; ++i)
				{
					if (HuntState.RoundPlayerState[i] != PLAYER_STATE_INVALID && HuntState.PlayerHunterCounter[i] < HUNT_MAX_HUNTER_COUNT)
						break;
				}

				// all players have reached the max number of times to be hunter
				if (i == GAME_MAX_PLAYERS)
				{
					HuntState.GameOver = 1;
				}

				// winner has reached score
				if (killsToWin > 0 && highestScore >= killsToWin)
				{
					HuntState.GameOver = 1;
				}
			}
		}
		else
		{
			// Display hello
			if ((gameTime - HuntState.RoundStartTicks) < (5 * TIME_SECOND))
			{
				for (i = 0; i < GAME_MAX_PLAYERS; ++i)
				{
					Player * p = players[i];
					if (!p || !playerIsLocal(p))
						continue;

					if (HuntState.RoundPlayerState[i] == PLAYER_STATE_DUCK_ALIVE)
						uiShowPopup(p->LocalPlayerIndex, HUNT_DUCK_HELLO);
					else if (HuntState.RoundPlayerState[i] == PLAYER_STATE_HUNTER)
						uiShowPopup(p->LocalPlayerIndex, HUNT_HUNTER_HELLO);
				}
			}

			// force game time
			if (HuntState.GameOver)
				gameData->TimeEnd = -1;
			else
				gameData->TimeEnd = (HuntState.RoundStartTicks - gameData->TimeStart) + (HUNT_ROUND_TIMELIMIT * TIME_SECOND);

			// host specific logic
			if (HuntState.IsHost && (gameGetTime() - HuntState.RoundStartTicks) > (5 * TIME_SECOND))
			{
				// determine game state
				int ducksAlive = 0, playerCount = 0, ducksFinished = 0;
				for (i = 0; i < GAME_MAX_PLAYERS; ++i)
				{
					switch (HuntState.RoundPlayerState[i])
					{
						case PLAYER_STATE_DUCK_FINISHED:
							++ducksFinished;
						case PLAYER_STATE_DUCK_ALIVE:
							++ducksAlive;
						case PLAYER_STATE_DUCK_DEAD:
						case PLAYER_STATE_HUNTER:
							++playerCount;
							break;
						default: break;
					}
				}

				// iterate each player
				for (i = 0; i < GAME_MAX_PLAYERS; ++i)
				{
					Player * p = players[i];

					if (p && HuntState.RoundPlayerState[i] != PLAYER_STATE_HUNTER)
					{
						// check if duck is dead
						if (playerIsDead(p) || HuntState.RoundPlayerState[i] == PLAYER_STATE_DUCK_DEAD)
						{
							// duck unfinished and newly died
							if (HuntState.RoundPlayerState[i] == PLAYER_STATE_DUCK_ALIVE)
							{
								DPRINTF("player %d died\n", i);
								HuntState.RoundPlayerState[i] = PLAYER_STATE_DUCK_DEAD;
							}
						}
						else if (HuntState.RoundPlayerState[i] == PLAYER_STATE_DUCK_ALIVE)
						{
							// get vector delta from end position
							vector_subtract(vTemp, DuckFinishPos, p->PlayerPosition);
							vTemp[3] = 0;
							float distance = vector_length(vTemp);

							// if duck is within bounds of finish then trigger finish
							if (distance <= DuckFinishPos[3])
							{
								onPlayerFinished(i);
							}
						}
					}
				}

				if (ducksAlive == 0 || ducksFinished == ducksAlive || (gameTime - HuntState.RoundStartTicks) > (HUNT_ROUND_TIMELIMIT * TIME_SECOND))
				{
					// end
					DPRINTF("end round: ducksAlive:%d ducksFinished:%d playerCount:%d\n", ducksAlive, ducksFinished, playerCount);
					setRoundOutcome();
				}
			}
		}
	}
	else
	{
		// set winner
		gameSetWinner(HuntState.WinningTeam, 0);

		// end game
		if (HuntState.GameOver == 1)
		{
			gameEnd(4);
			HuntState.GameOver = 2;
		}
	}

	// 
	updateScoreboard();

	return;
}




void setLobbyGameOptions(void)
{
	// deathmatch options
	static char options[] = { 
		0, 0, 			// 0x06 - 0x08
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
	int i;
	char buf[24];

	// reset buf
	memset(buf, 0, sizeof(buf));

	// sort scoreboard again
	sortScoreboard(1);

	// names start at 6
	// column headers start at 17
	strncpy((char*)(uiElements[18] + 0x60), "POINTS", 7);
	strncpy((char*)(uiElements[19] + 0x60), "KILLS", 6);
	strncpy((char*)(uiElements[20] + 0x60), "DEATHS", 7);
	strncpy((char*)(uiElements[21] + 0x60), "BEST TIME", 10);

	// rows
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		int pid = SortedPlayerScores[i]->TeamId;
		if (pid >= 0)
		{
			// set points to kills (removes suicides)
			sprintf(buf, "%d", SortedPlayerScores[i]->Value);
			strncpy((char*)(uiElements[22 + (i*4) + 0] + 0x60), buf, strlen(buf) + 1);

			// moves deaths over
			strncpy((char*)(uiElements[22 + (i*4) + 2] + 0x60), (char*)(uiElements[22 + (i*4) + 1] + 0x60), 8);
			
			// calculate kills as total score minus points by finishing
			sprintf(buf, "%d", SortedPlayerScores[i]->Value - HuntState.PlayerScore[pid]);
			strncpy((char*)(uiElements[22 + (i*4) + 1] + 0x60), buf, strlen(buf) + 1);

			// write best time
			int pTime = HuntState.PlayerBestTime[pid];
			if (pTime == 0)
				sprintf(buf, "DNF");
			else
				sprintf(buf, "%02d:%02d", pTime / TIME_MINUTE, (pTime % TIME_MINUTE) / TIME_SECOND);
			strncpy((char*)(uiElements[22 + (i*4) + 3] + 0x60), buf, strlen(buf) + 1);
		}
	}
}

/*
 * NAME :		lobbyStart
 * 
 * DESCRIPTION :
 * 			Duck hunt lobby logic entrypoint.
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
