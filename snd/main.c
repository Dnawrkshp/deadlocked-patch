/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Handles all search and destroy logic.
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
#include "module.h"

#include "include/pvars.h"

// TODO
// Create scoreboard abstraction in libdl

/*
 * When non-zero, it refreshes the in-game scoreboard.
 */
#define GAME_SCOREBOARD_REFRESH_FLAG        (*(u32*)0x002F9FC8)

/*
 * Target scoreboard value.
 */
#define GAME_SCOREBOARD_TARGET              (*(u32*)0x002FA084)

/*
 * Target scoreboard value.
 */
#define GAME_SCOREBOARD_NODE_TARGET         (*(u32*)0x00310180)

/*
 * Collection of scoreboard items.
 */
#define GAME_SCOREBOARD_ARRAY               ((ScoreboardItem**)0x002FA04C)

/*
 * Number of items in the scoreboard.
 */
#define GAME_SCOREBOARD_ITEM_COUNT          (*(u32*)0x002F9FCC)

/*
 * Number of rounds to win.
 */
#define SND_ROUNDS_TO_WIN					(6)

/*
 * Number of rounds before flipping team roles.
 */
#define SND_ROUNDS_TO_FLIP					(3)

/*
 * Max number of rounds before game ends
 */
#define SND_MAX_ROUNDS						(2 * (SND_ROUNDS_TO_WIN-1))

/*
 * Timelimit of each round in seconds.
 */
#define SND_ROUND_TIMELIMIT_SECONDS			(2 * 60)

/*
 * Number of seconds after bomb planted before explosion.
 */
#define SND_BOMB_TIMER_SECONDS				(3)

/*
 *
 */
#define SND_BOMB_TIMER_TEXT_SCALE			(3)

/*
 *
 */
#define SND_TEAM_DEFENDER_ID				(0)
#define SND_TEAM_ATTACKER_ID				(1)

/*
 *
 */
#define SND_BOMB_TIMER_BASE_COLOR1			(0xFFB700B7)
#define SND_BOMB_TIMER_BASE_COLOR2			(0xFF670067)
#define SND_BOMB_TIMER_HIGH_COLOR			(0xFFFFFFFF)

/*
 * Amount of time after a round ends before switching to next round.
 */
#define SND_ROUND_TRANSITION_WAIT_MS		(5 * TIME_SECOND)

/*
 * Popup strings
 */
const char * SND_BOMB_YOU_PICKED_UP = "You picked up the bomb!";
const char * SND_BOMB_PICKED_UP = "The bomb has been picked up!";
const char * SND_BOMB_PLANTED = "The bomb has been planted!";
const char * SND_BOMB_DEFUSED = "Bomb defused!";
const char * SND_BOMB_DROPPED = "The bomb has been dropped!";
const char * SND_ROUND_WIN = "Round win!";
const char * SND_ROUND_LOSS = "Round loss!";
const char * SND_HALF_TIME = "Switching sides...";
const char * SND_DEFEND_HELLO = "Defend your bombsites!";
const char * SND_ATTACK_HELLO = "Destroy the enemy bombsite!";

/*
 *
 */
typedef struct SNDNodeState
{
	Moby * Moby;
	GuberMoby * GuberMoby;
	GuberMoby * OrbGuberMoby;
} SNDNodeState_t;

/*
 *
 */
typedef struct SNDPlayerState
{
	Player * Player;
	int IsBombCarrier;
	int IsDead;
} SNDPlayerState_t;

/*
 *
 */
typedef struct SNDTimerState
{
	int LastPlaySoundSecond;
	u32 Color;
} SNDTimerState_t;

/*
 *
 */
enum SNDOutcome
{
	SND_OUTCOME_INCOMPLETE = 0,
	SND_OUTCOME_TIME_END,
	SND_OUTCOME_BOMB_DETONATED,
	SND_OUTCOME_BOMB_DEFUSED,
	SND_OUTCOME_ATTACKERS_DEAD
};

/*
 *
 */
struct SNDState
{
	int RoundNumber;
	int RoundStartTicks;
	int RoundEndTicks;
	int RoundResult;
	int RoundInitialized;
	int RoundLastWinners;
	int TeamWins[2];
	int TeamRolesFlipped;
	int GameOver;
	int WinningTeam;
	int BombPlantSiteIndex;
	int BombPlantedTicks;
	int BombDefused;
	int DefenderTeamId;
	int AttackerTeamId;
	GuberMoby * BombPackGuber;
	Moby * BombPackMoby;
	Moby * RadarObjectiveMoby[2];
	Player * BombCarrier;
	
	SNDNodeState_t Nodes[2];
	SNDPlayerState_t Players[GAME_MAX_PLAYERS];
	SNDTimerState_t Timer;
} SNDState;

/*
 *
 */
ScoreboardItem TeamScores[2];

/*
 *
 */
int ScoreboardChanged = 0;

/*
 *
 */
int Initialized = 0;
int InitializedTime = 0;

/*
 *
 */
char shaBuffer;

/*
 * Needed when moving hacker orbs
 */
void * HackerOrbCollisionPointer;

/*
 * Spawn settings
 */
VECTOR DefendTeamSpawnPoint = { 268.386, 122.752, 103.479, 0.8 };
VECTOR AttackTeamSpawnPoint = { 519.269, 396.575, 106.727, -1.351 };
VECTOR Node1SpawnPoint = { 428.368, 239.646, 106.613, 0 };
VECTOR Node2SpawnPoint = { 411.456, 143.924, 105.344, 0 };
VECTOR PackSpawnPoint = { 526.056, 370.259, 107.271, 0 };

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
	Player * localPlayer = (Player*)0x00347AA0;

	// Update values
	TeamScores[0].Value = SNDState.TeamWins[0];
	TeamScores[1].Value = SNDState.TeamWins[1];

	// Force scoreboard to custom scoreboard values
	if (GAME_SCOREBOARD_ARRAY[0] != &TeamScores[localPlayer->Team])
	{
		GAME_SCOREBOARD_ARRAY[0] = &TeamScores[localPlayer->Team];
		ScoreboardChanged = 1;
	}

	// Force scoreboard to custom scoreboard values
	if (GAME_SCOREBOARD_ARRAY[1] != &TeamScores[!localPlayer->Team])
	{
		GAME_SCOREBOARD_ARRAY[1] = &TeamScores[!localPlayer->Team];
		ScoreboardChanged = 1;
	}

	// Set hud flags
	if (gameGetTime() > (InitializedTime + 50))
	{
		for (i = 0; i < 2; ++i)
		{
			PlayerHUDFlags * hud = hudGetPlayerFlags(i);
			if (!hud->Flags.NormalScoreboard)
			{
				hud->Flags.ConquestScoreboard = 0;
				hud->Flags.NormalScoreboard = 1;
				hud->UNK_00[1] = 1; // refresh
			}
		}
	}
}

void setRoundOutcome(int outcome)
{
	// don't allow overwriting existing outcome
	if (SNDState.RoundResult)
		return;

	// 
	SNDState.RoundResult = outcome;
	SNDState.RoundEndTicks = gameGetTime() + SND_ROUND_TRANSITION_WAIT_MS;

	// print halftime message
	if ((SNDState.RoundNumber+1) % SND_ROUNDS_TO_FLIP == 0)
	{
		uiShowPopup(0, SND_HALF_TIME);
		uiShowPopup(1, SND_HALF_TIME);
	}

	DPRINTF("outcome set to %d\n", outcome);
}

void hideMoby(Moby * moby)
{
	if (!moby)
		return;

	moby->CollisionPointer = NULL;
	moby->RenderDistance = 0;
	memset(moby->Position, 0, sizeof(VECTOR));
	// DPRINTF("moby hidden (%d) at %08x\n", moby->MobyId, (u32)moby);
}

void hideNode(Moby * nodeBaseMoby, int keepNode, int keepOrb)
{
	int i = 2;
	NodeBasePVar_t * nodePvars = (NodeBasePVar_t*)nodeBaseMoby->PropertiesPointer;
	Moby * orb = nodePvars->HackerOrbMoby;
	Moby ** subItems = (Moby**)nodePvars->ChildMobies;

	// hide base and orb
	if (!keepNode)
	{
		hideMoby(nodeBaseMoby);
		i = 0;
	}

	if (!keepOrb)
	{
		hideMoby(orb);
	}

	// hide subitems (turrets)
	for (; i < 4; ++i)
		hideMoby(subItems[i]);
}

void moveNode(Moby * nodeBaseMoby, VECTOR position)
{
	int i = 1;
	NodeBasePVar_t * nodePvars = (NodeBasePVar_t*)nodeBaseMoby->PropertiesPointer;
	Moby * orb = nodePvars->HackerOrbMoby;
	Moby ** subItems = (Moby**)nodePvars->ChildMobies;

	vector_copy(nodeBaseMoby->Position, position);
	nodeBaseMoby->UNK_2C[2] |= 1;
	nodeBaseMoby->UNK_34[0] &= ~4;
	nodeBaseMoby->RenderDistance = 0xFF;
	if (orb)
	{
		vector_copy(orb->Position, position);
		orb->RenderDistance = 0xFF;
		orb->CollisionPointer = HackerOrbCollisionPointer;
	}

	for (i = 0; i < 2; ++i)
		if (subItems[i])
			vector_copy(subItems[i]->Position, position);
}

void hideNodes(void)
{
	Moby * moby = mobyGetFirst();
	
	while (moby)
	{
		Moby * next = moby->NextMoby;

		if (moby->MobyId == MOBY_ID_NODE_BASE)
		{
			int isBombSite = moby == SNDState.Nodes[0].Moby || moby == SNDState.Nodes[1].Moby;
			hideNode(moby, isBombSite, isBombSite);

			// Move node
			if (isBombSite)
			{
				if (moby == SNDState.Nodes[0].Moby)
					moveNode(moby, Node1SpawnPoint);
				else
					moveNode(moby, Node2SpawnPoint);
			}
		}
		else if (  moby->MobyId == MOBY_ID_BLUE_TEAM_HEALTH_PAD 
				|| moby->MobyId == MOBY_ID_PLAYER_TURRET 
				|| moby->MobyId == MOBY_ID_PICKUP_PAD
				|| moby->MobyId == MOBY_ID_CONQUEST_TURRET_HOLDER_TRIANGLE_THING
				|| moby->MobyId == MOBY_ID_CONQUEST_NODE_TURRET
				|| moby->MobyId == MOBY_ID_CONQUEST_POWER_TURRET
				|| moby->MobyId == MOBY_ID_CONQUEST_ROCKET_TURRET
				)
		{
			hideMoby(moby);
		}

		moby = next;
	}
}

void nodeCapture(GuberMoby * guberMoby, int team)
{
	u32 buffer[] = { team, -1 };
	
	GuberEvent * event = guberEventCreateEvent(&guberMoby->Guber, 2, 0, 0);
	if (event)
	{
		guberEventWrite(event, buffer + 0, 4);
		guberEventWrite(event, buffer + 1, 4);
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

	*(u8*)((u32)player + 0x2ed6) = 0; // turn off blown up state
	playerRespawn(player);
	playerSetPosRot(player, pos, r);
}

void replaceString(int textId, const char * str)
{
	// Get pointer to game string
	char * strPtr = uiMsgString(textId);
	strncpy(strPtr, str, 32);
}

void SNDHideMobyEventHandler(Moby * moby, GuberEvent * event, MobyEventHandler_func eventHandler)
{
	eventHandler(moby, event);
	hideMoby(moby);
}

void SNDNodeBaseEventHandler(Moby * moby, GuberEvent * event, MobyEventHandler_func eventHandler)
{
	u32 eventId = event->NetEvent[0] & 0xF;
	if (eventId == 0)
		hideNodes();

	eventHandler(moby, event);
}

void SNDHackerOrbEventHandler(Moby * moby, GuberEvent * event, MobyEventHandler_func eventHandler)
{
	int nodeIndex = -1;
	HackerOrbPVar_t * orbPvars = (HackerOrbPVar_t*)moby->PropertiesPointer;
	
	/*
	DPRINTF("Hacker Orb Event: %08x\n\t", (u32)moby);
	u32 * buffer = (u32*)event->NetEvent;
	for (i = 0; i < sizeof(event->NetEvent)/4; ++i)
		DPRINTF("%08X ", buffer[i]);
	DPRINTF("\n");

	DPRINTF("\n\tNetSendTime: %d\n\tNetSendTo: %d\n\tNetDataOffset: %02x\n\tMsgSendPending: %d\n\tNextEvent: %08x\n",
		event->NetSendTime,
		event->NetSendTo,
		event->NetDataOffset,
		event->MsgSendPending,
		(u32)event->NetEvent
		);
	*/

	// 
	u32 nodeUid = orbPvars->NodeBaseUid;
	if (SNDState.Nodes[0].GuberMoby->Guber.Id.UID == nodeUid)
	{
		nodeIndex = 0;
		SNDState.Nodes[0].OrbGuberMoby = moby->GuberMoby;
	}
	else if (SNDState.Nodes[1].GuberMoby->Guber.Id.UID == nodeUid)
	{
		nodeIndex = 1;
		SNDState.Nodes[1].OrbGuberMoby = moby->GuberMoby;
	}

	// update collision pointer
	if (!HackerOrbCollisionPointer)
		HackerOrbCollisionPointer = moby->CollisionPointer;

	// get id of event
	u32 eventId = event->NetEvent[0] & 0xF;

	// handle event
	switch (eventId)
	{
		case 2: // capture
		{
			// get capture team
			int team = *(int*)(event->NetEvent + 12);
			int playerId = *(int*)(event->NetEvent + 16);

			// Only capture if bomb is picked up
			if (SNDState.RoundInitialized && playerId >= 0 && !SNDState.BombPackMoby && nodeIndex >= 0)
			{
				if (team == SNDState.AttackerTeamId)
				{
					// bomb has been planted
					uiShowPopup(0, SND_BOMB_PLANTED);
					uiShowPopup(1, SND_BOMB_PLANTED);

					// change capture time to default (defuse time)
					*(u16*)0x00440E68 = 0x3C23;

					// set state
					SNDState.BombPlantedTicks = gameGetTime();
					SNDState.BombPlantSiteIndex = nodeIndex;

					// remove hacker ray from bomb holder
					if (playerId >= 0 && playerId < GAME_MAX_PLAYERS)
					{
						SNDState.Players[playerId].IsBombCarrier = 0;
						PlayerWeaponData * wepData = playerGetWeaponData(playerId);
						wepData[WEAPON_ID_HACKER_RAY].Level = -1;
					}

					// hide the other bomb site
					hideNode(SNDState.Nodes[!nodeIndex].Moby, 0, 0);
				}
				else
				{
					// bomb defused
					uiShowPopup(0, SND_BOMB_DEFUSED);
					uiShowPopup(1, SND_BOMB_DEFUSED);

					// set state
					setRoundOutcome(SND_OUTCOME_BOMB_DEFUSED);
					SNDState.BombDefused = 1;
				}
			}
			break;
		}
		case 1: // update
		{
			eventHandler(moby, event);

			if (nodeIndex >= 0)
			{
				int nodeTeam = orbPvars->NodeTeam;
				if (nodeTeam != SNDState.DefenderTeamId && nodeTeam != SNDState.AttackerTeamId)
					nodeCapture(moby->GuberMoby, SNDState.DefenderTeamId);
			}

			return;
		}
	}

	eventHandler(moby, event);
}

void SNDWeaponPackEventHandler(Moby * moby, GuberEvent * event, MobyEventHandler_func eventHandler)
{
	Player ** players = playerGetAll();
	Player * p;
	Player * localPlayer = (Player*)0x347AA0;
	int i;

	int eventId = event->NetEvent[0] & 0xF;
	int isNew = ((u32*)event->NetEvent)[2] == *(u32*)(*(u32*)0x00220710);

	// get id of event
	// 0 is pickup
	if (eventId == 0 && !isNew)
	{
		// get host id of player picking up pack
		int hostId = event->NetEvent[15] >> 4;

		// find player with hostId
		for (i = 0; i < GAME_MAX_PLAYERS; ++i)
		{
			p = *players;
			if (p && p->Guber.Id.GID.HostId == hostId)
			{
				// only allow attacking team to pickup
				if (p->Team != SNDState.AttackerTeamId)
					return;
				
				// 
				if (p != localPlayer && localPlayer->Team == SNDState.AttackerTeamId)
					uiShowPopup(localPlayer->LocalPlayerIndex, SND_BOMB_PICKED_UP);

				// remove reference
				SNDState.BombPackMoby = 0;
				SNDState.BombPackGuber = 0;

				// set bomb carrier
				SNDState.BombCarrier = p;
				for (i = 0; i < GAME_MAX_PLAYERS; ++i)
				{
					if (SNDState.Players[i].Player == p)
					{
						SNDState.Players[i].IsBombCarrier = 1;
						break;
					}
				}
				break;
			}
			++players;
		}
	}

	eventHandler(moby, event);
}

void GuberMobyEventHandler(Moby * moby, GuberEvent * event, MobyEventHandler_func eventHandler)
{
	switch (moby->MobyId)
	{
		case MOBY_ID_CONQUEST_HACKER_ORB: SNDHackerOrbEventHandler(moby, event, eventHandler); break;
		case MOBY_ID_WEAPON_PACK: SNDWeaponPackEventHandler(moby, event, eventHandler); break;
		case MOBY_ID_NODE_BASE: SNDNodeBaseEventHandler(moby, event, eventHandler); break;
		case MOBY_ID_CONQUEST_TURRET_HOLDER_TRIANGLE_THING:
		case MOBY_ID_CONQUEST_NODE_TURRET:
		case MOBY_ID_CONQUEST_POWER_TURRET:
		case MOBY_ID_CONQUEST_ROCKET_TURRET:
			SNDHideMobyEventHandler(moby, event, eventHandler);
			break;
		default:
			DPRINTF("GuberMoby event (%04x) with %08x and %08x, handler=%08x\n", moby->MobyId, (u32)moby, (u32)event, (u32)eventHandler);
			eventHandler(moby, event);
			break;
	}
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
				
	return moby;
}

void * spawnPackHook(u16 mobyId, int pvarSize, int guberId, int arg4, int arg5)
{
	void * result = ((void* (*)(u16, int, int, int, int))0x0061C3A8)(mobyId, pvarSize, guberId, arg4, arg5);

	if (mobyId == MOBY_ID_WEAPON_PACK)
	{
		// only bomb pack can spawn
		SNDState.BombPackMoby = (Moby*)(*(u32*)((u32)result + 0x18));
		SNDState.BombPackMoby->TextureId = 0x80 + (8 * SNDState.AttackerTeamId);

		DPRINTF("spawnPackHook bomb pack moby = %08x\n", (u32)SNDState.BombPackMoby);
	}

	return result;
}

GuberMoby * spawnPackGuber(VECTOR position, u32 mask)
{
	GuberEvent * guberEvent = 0;
	Player * localPlayer = (Player*)0x00347AA0;
	VECTOR unk = {0,0,-1,0};
	int zero = 0;

	// create guber object
	GuberMoby * guberMoby = guberMobyCreateSpawned(MOBY_ID_WEAPON_PACK, 0x40, &guberEvent, &localPlayer->Guber);
	if (guberEvent)
	{
		guberEventWrite(guberEvent, position, 0x0C);
		guberEventWrite(guberEvent, unk, 0x0C);
		guberEventWrite(guberEvent, &mask, 4);
		guberEventWrite(guberEvent, &zero, 4);
	}

	return guberMoby;
}

void drawRoundMessage(const char * message, float scale)
{
	u32 boxColor = 0x20ffffff;
	int fw = gfxGetFontWidth(message, -1, scale);
	float w = fw / (2.0 * SCREEN_WIDTH);
	float h = 36.0 / SCREEN_HEIGHT;
	float x = 0.5;
	float y = 0.14;
	float yOff = -0.015;
	float p = 0.02;
	RECT r = { {x-w-p,y-p}, {x+w+p,y-p}, {x-w-p,y+h+p}, {x+w+p,y+h+p} };

	// draw container
	gfxScreenSpaceBox(&r, boxColor, boxColor, boxColor, boxColor);

	// draw message
	gfxScreenSpaceText(SCREEN_WIDTH * x, SCREEN_HEIGHT * (y + yOff), scale, scale * 1.5, 0x80FFFFFF, message, -1);
}

void playTimerTickSound()
{
	((void (*)(Player*, int, int))0x005eb280)((Player*)0x347AA0, 0x3C, 0);
}

void bombTimerLogic()
{
	int gameTime = gameGetTime();
	char strBuf[16];
	int i;
	
	if (!SNDState.BombDefused && SNDState.BombPlantedTicks > 0 && SNDState.BombPlantSiteIndex >= 0)
	{
		SNDNodeState_t * plantSiteNodeState = &SNDState.Nodes[SNDState.BombPlantSiteIndex];
		int timeLeft = (SND_BOMB_TIMER_SECONDS * TIME_SECOND) - (gameTime - SNDState.BombPlantedTicks);
		float timeSecondsLeft = timeLeft / (float)TIME_SECOND;
		float scale = SND_BOMB_TIMER_TEXT_SCALE;
		u32 color = 0xFFFFFFFF;
		int timeSecondsLeftFloor = (int)timeSecondsLeft;
		float timeSecondsRounded = timeSecondsLeftFloor;
		if ((timeSecondsLeft - timeSecondsRounded) > 0.5)
			timeSecondsRounded += 1;


		if (timeLeft <= 0)
		{
			// set end
			setRoundOutcome(SND_OUTCOME_BOMB_DETONATED);

			// detonate
			for (i = 0; i < 5; ++i)
				spawnExplosion(plantSiteNodeState->OrbGuberMoby->Moby->Position, 5);

			// blow up node
			hideNode(plantSiteNodeState->Moby, 1, 0);

			// blow up defenders
			for (i = 0; i < GAME_MAX_PLAYERS; ++i)
			{
				SNDPlayerState_t * player = &SNDState.Players[i];
				if (player->Player)
				{
					if (player->Player->Team == SNDState.DefenderTeamId)
					{
						player->Player->BlowupTimer = 1;
					}
				}
			}
		}
		else
		{
			// update scale
			float t = 1-fabsf(timeSecondsRounded - timeSecondsLeft);
			float x = powf(t, 15);
			scale *= (1.0 + (0.3 * x));

			// update color
			color = lerpColor(SND_BOMB_TIMER_BASE_COLOR1, SND_BOMB_TIMER_BASE_COLOR2, fabsf(sinf(gameTime * 10)));
			color = lerpColor(color, SND_BOMB_TIMER_HIGH_COLOR, x);

			// draw timer
			sprintf(strBuf, "%.02f", timeLeft / (float)TIME_SECOND);
			gfxScreenSpaceText(SCREEN_WIDTH/2, SCREEN_HEIGHT * 0.14, scale, scale, color, strBuf, -1);

			// tick timer
			if (timeSecondsLeftFloor < SNDState.Timer.LastPlaySoundSecond)
			{
				SNDState.Timer.LastPlaySoundSecond = timeSecondsLeftFloor;
				playTimerTickSound();
			}
		}
	}
}

void playerLogic(SNDPlayerState_t * playerState, int isHost)
{
	Player * localPlayer = (Player*)0x00347AA0;
	if (!playerState->Player)
		return;
	
	// Check if died
	if (!playerState->IsDead && playerState->Player->Health <= 0)
	{
		// spawn new bomb on bomb carrier death
		if (playerState->IsBombCarrier)
		{
			playerState->IsBombCarrier = 0;
			SNDState.BombCarrier = 0;

			// spawn new pack if host
			if (isHost)
				SNDState.BombPackGuber = (GuberMoby*)spawnPackGuber(playerState->Player->PlayerPosition, 1 << WEAPON_ID_HACKER_RAY);

			// tell team bomb has dropped
			if (localPlayer->Team == SNDState.AttackerTeamId)
				uiShowPopup(0, SND_BOMB_DROPPED);
		}

		playerState->IsDead = 1;
	}
}

void resetRoundState(void)
{
	int i;
	Player ** players = playerGetAll();
	Player * player = NULL;
	Player * localPlayer = (Player*)0x00347AA0;
	GameData * gameData = gameGetData();
	int gameTime = gameGetTime();

	// 
	SNDState.RoundInitialized = 0;
	SNDState.RoundEndTicks = 0;
	SNDState.RoundStartTicks = gameTime;
	SNDState.RoundResult = SND_OUTCOME_INCOMPLETE;
	SNDState.BombDefused = 0;
	SNDState.BombPlantSiteIndex = -1;
	SNDState.BombPlantedTicks = 0;
	SNDState.BombCarrier = 0;

	// 
	SNDState.Timer.LastPlaySoundSecond = SND_BOMB_TIMER_SECONDS;
	SNDState.Timer.Color = 0xFFFFFFFF;

	// Set round time limit
	gameData->TimeEnd = (gameTime - gameData->TimeStart) + (SND_ROUND_TIMELIMIT_SECONDS * TIME_SECOND);
	
	// set capture time to fast (plant speed)
	*(u16*)0x00440E68 = 0x3CA3;

	// iterate players
	for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
		player = players[i];
		
		// update state
		SNDState.Players[i].Player = player;
		SNDState.Players[i].IsDead = 0;
		SNDState.Players[i].IsBombCarrier = 0;
		
		// Remove hacker rays
		if (player)
		{
			if (player->Team == SNDState.AttackerTeamId)
			{
				spawnPlayer(player, AttackTeamSpawnPoint);

				// remove hacker ray from attackers
				PlayerWeaponData * wepData = playerGetWeaponData(player->PlayerId);
				wepData[WEAPON_ID_HACKER_RAY].Level = -1;
			}
			else
			{
				spawnPlayer(player, DefendTeamSpawnPoint);
			}
		}
	}

	// Give nodes to defending team
	if (Initialized)
	{
		// move
		moveNode(SNDState.Nodes[0].Moby, Node1SpawnPoint);
		moveNode(SNDState.Nodes[1].Moby, Node2SpawnPoint);

		// capture
		nodeCapture(SNDState.Nodes[0].OrbGuberMoby, SNDState.DefenderTeamId);
		nodeCapture(SNDState.Nodes[1].OrbGuberMoby, SNDState.DefenderTeamId);
	}

	// 
	if (SNDState.BombPackMoby)
	{
		mobyDestroy(SNDState.BombPackMoby);
		SNDState.BombPackMoby = NULL;
	}

	// spawn hacker ray pack
	if (gameIsHost(localPlayer->Guber.Id.GID.HostId))
	{
		SNDState.BombPackGuber = (GuberMoby*)spawnPackGuber(PackSpawnPoint, 1 << WEAPON_ID_HACKER_RAY);
	}

	SNDState.RoundInitialized = 1;
}

/*
 * NAME :		initialize
 * 
 * DESCRIPTION :
 * 			Initializes the game mode.
 * 			Resets states, generates random weapon ordering, generates random alpha mods.
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

	// Reset snd state
	SNDState.RoundNumber = 0;
	SNDState.TeamWins[0] = 0;
	SNDState.TeamWins[1] = 0;
	SNDState.TeamRolesFlipped = 0;
	SNDState.GameOver = 0;
	SNDState.Nodes[0].Moby = 0;
	SNDState.Nodes[1].Moby = 0;
	SNDState.Nodes[0].GuberMoby = 0;
	SNDState.Nodes[1].GuberMoby = 0;
	SNDState.Nodes[0].OrbGuberMoby = 0;
	SNDState.Nodes[1].OrbGuberMoby = 0;
	SNDState.BombPackMoby = 0;
	SNDState.BombPackGuber = 0;
	SNDState.DefenderTeamId = TEAM_BLUE;
	SNDState.AttackerTeamId = TEAM_RED;

	// Install spawn pack hook
	*(u32*)0x0061CDC8 = 0x0C000000 | ((u32)&spawnPackHook / 4);

	// Enable hacker ray in packs
	*(u8*)0x00413DB4 = 40;

	// Disable cq popup messages
	*(u32*)0x003D2E6C = 0;

	// Disable normal game ending
	*(u32*)0x006219B8 = 0;	// survivor (8)
	*(u32*)0x00620F54 = 0;	// time end (1)
	*(u32*)0x00621240 = 0;	// homenode (4)


	// Overwrite 'you picked up a weapon pack' string to pickup bomb message
	replaceString(0x2331, SND_BOMB_YOU_PICKED_UP);

	// Initialize scoreboard
	for (i = 0; i < 2; ++i)
	{
		TeamScores[i].TeamId = i;
		TeamScores[i].UNK = 0;
		TeamScores[i].Value = 0;
	}

	// 
	ScoreboardChanged = 1;

	// Set kill target to 0 (disable)
	gameSetKillsToWin(0);

	// Disable packs
	cheatsApplyNoPacks();

	// Enable survivor
	gameSetSurvivor(1);

	// 
	gameSetRespawnTime(0xFF);

	// 
	SNDState.RadarObjectiveMoby[0] = mobySpawn(MOBY_ID_BETA_BOX, 0);
	SNDState.RadarObjectiveMoby[0]->ModelPointer = 0;
	SNDState.RadarObjectiveMoby[0]->CollisionPointer = 0;
	SNDState.RadarObjectiveMoby[1] = mobySpawn(MOBY_ID_BETA_BOX, 0);
	SNDState.RadarObjectiveMoby[1]->ModelPointer = 0;
	SNDState.RadarObjectiveMoby[1]->CollisionPointer = 0;

	// Write patch to hook GuberMoby event handler
	*(u32*)0x0061CB30 = 0x8C460014; // move func ptr to a2
	*(u32*)0x0061CB38 = 0x0C000000 | ((u32)&GuberMobyEventHandler / 4); // call our func

	// Use gubers to find our nodes
	while (guberMoby)
	{
		GuberMoby * next = (GuberMoby*)guberMoby->Guber.Prev;

		if (guberMoby->Moby && guberMoby->Moby->MobyId == MOBY_ID_NODE_BASE)
		{
			if (guberMoby->Guber.Id.UID == 0xF6000002)
			{
				SNDState.Nodes[0].GuberMoby = guberMoby;
				SNDState.Nodes[0].Moby = guberMoby->Moby;
				DPRINTF("Node1: %08x\n", (u32)guberMoby->Moby);
			}
			else if (guberMoby->Guber.Id.UID == 0xF6000003)
			{
				SNDState.Nodes[1].GuberMoby = guberMoby;
				SNDState.Nodes[1].Moby = guberMoby->Moby;
				DPRINTF("Node2: %08x\n", (u32)guberMoby->Moby);
			}
		}

		guberMoby = next;
	}
	
	// Disable all other nodes
	hideNodes();

	// reset snd round state
	resetRoundState();

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
	GameData * gameData = gameGetData();
	int gameTime = gameGetTime();

	// Ensure in game
	if (!gameSettings)
		return;

	if (!Initialized)
		initialize();

#if DEBUG
	if (!SNDState.GameOver && padGetButton(0, PAD_L3 | PAD_R3) > 0)
		SNDState.GameOver = 1;
#endif

	if (!gameHasEnded() && gameIsIn() && !SNDState.GameOver)
	{
		if (SNDState.RoundEndTicks)
		{
			// Handle game outcome
			if (SNDState.RoundResult)
			{
				switch (SNDState.RoundResult)
				{
					case SND_OUTCOME_TIME_END:
					case SND_OUTCOME_ATTACKERS_DEAD:
					case SND_OUTCOME_BOMB_DEFUSED:
					{
						// defenders win
						if (++SNDState.TeamWins[SNDState.DefenderTeamId] >= SND_ROUNDS_TO_WIN)
							SNDState.GameOver = 1;
						
						SNDState.RoundLastWinners = SNDState.DefenderTeamId;
						break;
					}
					case SND_OUTCOME_BOMB_DETONATED:
					{
						// attackers win
						if (++SNDState.TeamWins[SNDState.AttackerTeamId] >= SND_ROUNDS_TO_WIN)
							SNDState.GameOver = 1;

						SNDState.RoundLastWinners = SNDState.AttackerTeamId;
						break;
					}
				}

				// update current winning team
				SNDState.WinningTeam = -1;
				if (SNDState.TeamWins[SNDState.DefenderTeamId] > SNDState.TeamWins[SNDState.AttackerTeamId])
					SNDState.WinningTeam = SNDState.DefenderTeamId;
				else if (SNDState.TeamWins[SNDState.DefenderTeamId] < SNDState.TeamWins[SNDState.AttackerTeamId])
					SNDState.WinningTeam = SNDState.AttackerTeamId;
				
				ScoreboardChanged = 1;
				SNDState.RoundResult = SND_OUTCOME_INCOMPLETE;
			}

			if (gameTime > SNDState.RoundEndTicks)
			{
				// increment round counter
				SNDState.RoundNumber += 1;

				if (SNDState.RoundNumber >= SND_MAX_ROUNDS)
				{
					// reach max number of rounds
					// game must be a draw
					SNDState.WinningTeam = -1;
					SNDState.GameOver = 1;
				}
				else
				{
					// handle half time
					if (SNDState.RoundNumber == SND_ROUNDS_TO_FLIP)
					{
						SNDState.TeamRolesFlipped = !SNDState.TeamRolesFlipped;
						SNDState.DefenderTeamId = !SNDState.DefenderTeamId;
						SNDState.AttackerTeamId = !SNDState.AttackerTeamId;
					}

					// reset
					resetRoundState();
				}
			}
			else
			{
				if (localPlayer->Team == SNDState.RoundLastWinners)
					drawRoundMessage(SND_ROUND_WIN, 1.5);
				else
					drawRoundMessage(SND_ROUND_LOSS, 1.5);
			}
		}
		else
		{
			// Set lifetime of bomb pack moby
			if (SNDState.BombPackMoby)
			{
				if (SNDState.BombPackMoby->MobyId != MOBY_ID_WEAPON_PACK)
				{
					SNDState.BombPackMoby = NULL;
				}
				else if (SNDState.BombPackMoby->PropertiesPointer)
				{
					// set lifetime to max
					*(u32*)((u32)SNDState.BombPackMoby->PropertiesPointer + 0x8) = 0x01ffffff;
				}
			}

			// End round
			if (gameData->TimeEnd && gameTime > gameData->TimeEnd)
			{
				setRoundOutcome(SND_OUTCOME_TIME_END);
			}

			// Display hello
			if ((SNDState.RoundNumber % SND_ROUNDS_TO_FLIP) == 0 && (gameTime - SNDState.RoundStartTicks) < (5 * TIME_SECOND))
			{
				if (localPlayer->Team == SNDState.DefenderTeamId)
					drawRoundMessage(SND_DEFEND_HELLO, 1);
				else
					drawRoundMessage(SND_ATTACK_HELLO, 1);
			}

			// Draw objective
			Moby * target[2] = {0,0};

			// If bomb is planted, make objective bombsite for all
			if (SNDState.BombPlantSiteIndex >= 0)
			{
				target[0] = SNDState.Nodes[SNDState.BombPlantSiteIndex].Moby;
			}
			else if (localPlayer->Team == SNDState.DefenderTeamId || localPlayer == SNDState.BombCarrier)
			{
				// Defenders and bomb carrier's targets are the two nodes
				target[0] = SNDState.Nodes[0].Moby;
				target[1] = SNDState.Nodes[1].Moby;
			}
			else
			{
				// If attacker draw objective where bomb is
				if (SNDState.BombPackMoby)
				{
					target[0] = SNDState.BombPackMoby;
				}
				else if (SNDState.BombCarrier)
				{
					target[0] = SNDState.BombCarrier->PlayerMoby;
				}
			}

			for (i = 0; i < 2; ++i)
			{
				if (target[i])
				{
					int blipId = radarGetBlipIndex(SNDState.RadarObjectiveMoby[i]);
					if (blipId >= 0)
					{
						RadarBlip * blip = radarGetBlips() + blipId;
						blip->X = target[i]->Position[0];
						blip->Y = target[i]->Position[1];
						blip->Life = 0x1F;
						blip->Type = 0x11;
					}
				}
			}

			int isHost = gameIsHost(localPlayer->Guber.Id.GID.HostId);
			int attackersAlive = 0;
			int hasAttackers = 0;
			for (i = 0; i < GAME_MAX_PLAYERS; ++i)
			{
				if (SNDState.Players[i].Player && SNDState.Players[i].Player->Team == SNDState.AttackerTeamId)
				{
					hasAttackers = 1;
					if (!SNDState.Players[i].IsDead)
						attackersAlive = 1;
				}

				playerLogic(&SNDState.Players[i], isHost);
			}

#if DEBUG && FALSE
			//
			static int lastD = 0; 
			if ((gameTime - lastD) > TIME_SECOND)
			{
				lastD = gameTime;
				spawnExplosion(SNDState.Nodes[0].OrbGuberMoby->Moby->Position, 5);
			}
#endif


			// no attackers alive and bomb hasn't been planted
			if (hasAttackers && !attackersAlive && !SNDState.BombPlantedTicks)
				setRoundOutcome(SND_OUTCOME_ATTACKERS_DEAD);

			//
			bombTimerLogic();
		}
	}
	else
	{
		// Set lifetime of bomb pack moby to 0
		if (SNDState.BombPackMoby)
		{
			if (SNDState.BombPackMoby->MobyId == MOBY_ID_WEAPON_PACK && SNDState.BombPackMoby->PropertiesPointer)
				*(u32*)((u32)SNDState.BombPackMoby->PropertiesPointer + 0x8) = 0;

			SNDState.BombPackMoby = NULL;
		}

		// set winner
		gameSetWinner(SNDState.WinningTeam, 1);

		// end game
		if (SNDState.GameOver == 1)
		{
			gameEnd(4);
			SNDState.GameOver = 2;
		}
	}

	// 
	updateScoreboard();

	// Update scoreboard on change
	if (ScoreboardChanged)
	{
		GAME_SCOREBOARD_ITEM_COUNT = 2;
		GAME_SCOREBOARD_NODE_TARGET = SND_MAX_ROUNDS;
		GAME_SCOREBOARD_TARGET = SND_ROUNDS_TO_WIN;
		GAME_SCOREBOARD_REFRESH_FLAG = 1;
		ScoreboardChanged = 0;
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
