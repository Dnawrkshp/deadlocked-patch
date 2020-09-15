/***************************************************
 * FILENAME :		deadlocked.c
 * 
 * DESCRIPTION :
 * 		Manages and applies all Deadlocked patches.
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

void (* ForceGameEnd)(int) = (void(*)(int))0x006228C8;

void (* ShowPopup)(int, const char *) = (void(*)(int, const char *))0x00540170;

int (*sprintf)(char *, const char *, ...) = (int(*)(char *, const char *, ...))0x0011D920;

unsigned int (*strlen)(const char *) = (unsigned int(*)(const char *))0x0011AB04;

void (*memcpy)(void *, const void *, int) = (void(*)(void *, const void *, int))0x0011A370;

int (*sha1)(const void *, int, void *, int) = (int(*)(const void *, int, void *, int))0x01EB30C8;

int IsLocal(Player * player)
{
    return (int)player >= 0x00300000 && (int)player <= 0x00400000;
}

#endif

int WeaponSlotToId(int slotId)
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

PlayerWeaponData * GetPlayerWeaponData(int playerId)
{
    return (PlayerWeaponData *)(WEAPON_DATA_START + (WEAPON_DATA_SIZE * playerId));
}

void SetPlayerEquipslot(int playerId, int slot, int weaponId)
{
    int * equipslots = WEAPON_EQUIPSLOT;
    equipslots[slot + (playerId * 3)] = weaponId;
}

void GiveWeapon(Player * player, int weaponId, int weaponLevel)
{
    void (* giveWeapon)(u32, int, int, int) = (void (*)(u32, int, int, int))PLAYER_GIVEWEP_FUNCADD;

    giveWeapon(player->WeaponHeldDataPointer, weaponId, weaponLevel, 1);
}

/*
 * NAME :		ChangeWeapon
 * 
 * DESCRIPTION :
 * 			Change player's equipped weapon.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      player      :           Pointer to player data struct
 *      weaponId    :           Weapon id to set
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void ChangeWeapon(Player * player, int weaponId)
{
    if (!player)
        return;

    player->ChangeWeaponHeldId = weaponId;
}

/*
 * NAME :		ChangeTeam
 * 
 * DESCRIPTION :
 * 			Change player's equipped weapon.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      player      :           Pointer to player data struct
 *      teamId    :             Team id to set
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void ChangeTeam(Player * player, int teamId)
{
    if (!player)
        return;

    
    player->Team = teamId;
    player->PlayerMoby->SecondaryColor = TEAM_COLORS[teamId];
}

/*
 * NAME :		SetWinner
 * 
 * DESCRIPTION :
 * 			Set the winning team/player
 * 
 * NOTES :
 * 
 * ARGS : 
 *      teamOrPlayerId    :             Team/player id to set
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void SetWinner(int teamOrPlayerId)
{
    GAME_WINNER = teamOrPlayerId;
}

/*
 * NAME :		EndGame
 * 
 * DESCRIPTION :
 * 			End game.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      winner :        Winning team/player.
 *      isPlayer :      Whether or not a player or team won.
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void EndGame(int winner, int isPlayer)
{
    GAME_WINNER = winner;
    if (!GAME_HAS_ENDED)
        ForceGameEnd(isPlayer);
    GAME_WINNER = winner;
}

/*
 * NAME :		IsInGame
 * 
 * DESCRIPTION :
 * 			Whether the client is currently in a game.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int IsInGame(void)
{
    return GAME_ACTIVE;
}
