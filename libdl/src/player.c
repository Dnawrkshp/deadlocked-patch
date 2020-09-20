#include "player.h"


/*
 * 
 */
#define PLAYER_STRUCT_ARRAY                         ((Player**)0x00344C38)

/*
 * Local player 1 dme player index.
 */
#define PLAYER_1_ID                                 (*(u32*)0x0017218C)

/*
 * Local player 2 dme player index.
 */
#define PLAYER_2_ID                                 (*(u32*)0x001B6ED8)

/*
 * Weapon stuff.
 */
#define WEAPON_DATA_START                           (0x001D49C0)
#define WEAPON_DATA_SIZE                            (0x12B0)
#define WEAPON_EQUIPSLOT                            ((int*)0x0020C690)
#define PLAYER_GIVEWEP_FUNC                         (0x00628018)

/*
 * 
 */
#define PLAYER_SET_POS_ROT_FUNC                     (0x005EB448)

//--------------------------------------------------------------------------------
Player ** getPlayers(void)
{
    return PLAYER_STRUCT_ARRAY;
}

//--------------------------------------------------------------------------------
PlayerWeaponData * getPlayerWeaponData(int playerId)
{
    return (PlayerWeaponData *)(WEAPON_DATA_START + (WEAPON_DATA_SIZE * playerId));
}

//--------------------------------------------------------------------------------
void setLocalPlayerEquipslot(int localPlayerId, int slot, int weaponId)
{
    int * equipslots = WEAPON_EQUIPSLOT;
    equipslots[slot + (localPlayerId * 3)] = weaponId;
}

//--------------------------------------------------------------------------------
void giveWeapon(Player * player, int weaponId, int weaponLevel)
{
    ((void (*)(u32, int, int, int))PLAYER_GIVEWEP_FUNC)(player->WeaponHeldDataPointer, weaponId, weaponLevel, 1);
}

//--------------------------------------------------------------------------------
void changeWeapon(Player * player, int weaponId)
{
    if (!player)
        return;

    player->ChangeWeaponHeldId = weaponId;
}

//--------------------------------------------------------------------------------
void changeTeam(Player * player, int teamId)
{
    if (!player)
        return;

    
    player->Team = teamId;
    player->PlayerMoby->SecondaryColor = TEAM_COLORS[teamId];
}

//--------------------------------------------------------------------------------
int isLocal(Player * player)
{
    return (int)player >= 0x00300000 && (int)player <= 0x00400000;
}

//--------------------------------------------------------------------------------
void playerSetPosRot(Player * player, Vector3 * p, Vector3 * r)
{
    ((void (*)(Player *, Vector3 *, Vector3 *, int))PLAYER_SET_POS_ROT_FUNC)(player, p, r, 0);
}
