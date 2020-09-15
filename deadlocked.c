/***************************************************
 * FILENAME :		deadlocked.c
 * 
 * DESCRIPTION :
 * 		Implements all functions declared in include\*.h
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#include <tamtypes.h>
#include "common.h"

#include "appid.h"
#include "time.h"
#include "module.h"
#include "game.h"
#include "gamesettings.h"
#include "player.h"
#include "team.h"
#include "cheats.h"


const u32 TEAM_COLORS[] =
{
    0x40FF6000,         // blue
    0x400000FF,         // red
    0x4006FF00,         // green
    0x40008AFF,         // orange
    0x4000EAFF,         // yellow
    0x40FF00E4,         // purple
    0x40F0FF00,         // aqua
    0x40C674FF,         // pink
    0x4000FF9C,         // olive
    0x406000FF,         // maroon
    0x40FFFFFF          // white
};

#if APPID == DL_APPID

void (* forceGameEnd)(int) = (void(*)(int))0x006228C8;

void (* showPopup)(int, const char *) = (void(*)(int, const char *))0x00540170;

int (*sprintf)(char *, const char *, ...) = (int(*)(char *, const char *, ...))0x0011D920;

unsigned int (*strlen)(const char *) = (unsigned int(*)(const char *))0x0011AB04;

void (*memcpy)(void *, const void *, int) = (void(*)(void *, const void *, int))0x0011A370;

int (*sha1)(const void *, int, void *, int) = (int(*)(const void *, int, void *, int))0x01EB30C8;

#endif

/*
 * ------------------------------------------------
 * ----------------- START WEAPON -----------------
 * ------------------------------------------------
 */

int weaponSlotToId(int slotId)
{
    switch (slotId)
    {
        case WEAPON_SLOT_WRENCH: return WEAPON_ID_WRENCH;
        case WEAPON_SLOT_VIPERS: return WEAPON_ID_VIPERS;
        case WEAPON_SLOT_MAGMA_CANNON: return WEAPON_ID_MAGMA_CANNON;
        case WEAPON_SLOT_ARBITER: return WEAPON_ID_ARBITER;
        case WEAPON_SLOT_FUSION_RIFLE: return WEAPON_ID_FUSION_RIFLE;
        case WEAPON_SLOT_MINE_LAUNCHER: return WEAPON_ID_MINE_LAUNCHER;
        case WEAPON_SLOT_B6: return WEAPON_ID_B6;
        case WEAPON_SLOT_FLAIL: return WEAPON_ID_FLAIL;
        case WEAPON_SLOT_OMNI_SHIELD: return WEAPON_ID_OMNI_SHIELD;
        default: return WEAPON_ID_EMPTY;
    }
}

/*
 * ------------------------------------------------
 * ----------------- START PLAYER -----------------
 * ------------------------------------------------
 */

PlayerWeaponData * getPlayerWeaponData(int playerId)
{
    return (PlayerWeaponData *)(WEAPON_DATA_START + (WEAPON_DATA_SIZE * playerId));
}

void setLocalPlayerEquipslot(int localPlayerId, int slot, int weaponId)
{
    int * equipslots = WEAPON_EQUIPSLOT;
    equipslots[slot + (localPlayerId * 3)] = weaponId;
}

void giveWeapon(Player * player, int weaponId, int weaponLevel)
{
    void (* _giveWeapon)(u32, int, int, int) = (void (*)(u32, int, int, int))PLAYER_GIVEWEP_FUNCADD;

    _giveWeapon(player->WeaponHeldDataPointer, weaponId, weaponLevel, 1);
}

void changeWeapon(Player * player, int weaponId)
{
    if (!player)
        return;

    player->ChangeWeaponHeldId = weaponId;
}

void changeTeam(Player * player, int teamId)
{
    if (!player)
        return;

    
    player->Team = teamId;
    player->PlayerMoby->SecondaryColor = TEAM_COLORS[teamId];
}

int isLocal(Player * player)
{
    return (int)player >= 0x00300000 && (int)player <= 0x00400000;
}

/*
 * ------------------------------------------------
 * ------------------ START GAME ------------------
 * ------------------------------------------------
 */

void setWinner(int teamOrPlayerId, int isTeam)
{
    GAME_WINNER_TEAM_ID = teamOrPlayerId;
    GAME_WINNER_PLAYER_ID = isTeam ? -1 : teamOrPlayerId;
}

void endGame(int reason)
{
    if (!GAME_HAS_ENDED)
        forceGameEnd(reason);
}

int isInGame(void)
{
    return GAME_ACTIVE;
}


/*
 * ------------------------------------------------
 * ----------------- START CHEATS -----------------
 * ------------------------------------------------
 */

void cheatsApplyNoPacks(void)
{
    *CHEAT_NO_PACKS_START = 0x03E00008;
    *(CHEAT_NO_PACKS_START+1) = 0x00000000;
}

void cheatsApplyNoV2s(void)
{
    // Nop function call
    CHEAT_NO_V2S_JAL = 0;
}

int cheatsDisableHealthboxes(void)
{
    int count = 0;

    // Iterate through mobys and disable healthboxes
	Moby ** mobiesArray = MOBY_ARRAY;
	Moby * currentMoby;
	while ((currentMoby = *mobiesArray))
	{
		if (currentMoby->MobyId == MOBY_ID_HEALTH_BOX_MULT)
		{
			currentMoby->PositionX = 0;
			currentMoby->PositionY = 0;
			currentMoby->PositionZ = 0;

			if (currentMoby->PropertiesPointer)
			{
				void * subPtr = (void*)(*(u32*)(currentMoby->PropertiesPointer));
				if (subPtr)
				{
					Moby * orb = (Moby*)(*(u32*)(subPtr + 0x98));
					if (orb)
					{
						orb->PositionX = 0;
						orb->PositionY = 0;
						orb->PositionZ = 0;

						
						++count;
					}
				}
				// 
			}
		}

		++mobiesArray;
	}

    return count;
}


void cheatsApplyMirrorWorld(int isOn)
{
    CHEAT_MIRROR_WORLD = isOn ? 1 : 0;
}

void cheatsApplyColorblindMode(u8 mode)
{
    CHEAT_COLORBLIND_MODE = mode;
}

void cheatsApplyWeather(u8 weatherId)
{
    CHEAT_WEATHER_MODE = weatherId;
}

