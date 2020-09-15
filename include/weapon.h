/***************************************************
 * FILENAME :		weapon.h
 * 
 * DESCRIPTION :
 * 		Contains weapon specific offsets and structures for Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _WEAPON_H_
#define _WEAPON_H_

#include "appid.h"

#if APPID == DL_APPID

#define WEAPON_DATA_START                   (0x001D49C0)
#define WEAPON_DATA_SIZE                    (0x12B0)

#define WEAPON_EQUIPSLOT                    ((int*)0x0020C690)

#endif


typedef struct PlayerWeaponData
{
    short Level;
    short Ammo;

    int Experience;

    int GameTimeLastShot;
    
    int OmegaMod;

    int UNK_10;

    int AlphaMods[10];

    char UNK_3C[0x08];

} PlayerWeaponData;

/*
 * NAME :		WEAPON_IDS
 * 
 * DESCRIPTION :
 * 			Defines the value for each weapon id.
 * 
 * NOTES :
 *          
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
enum WEAPON_IDS
{
    WEAPON_ID_EMPTY =               0,
    WEAPON_ID_WRENCH =              1,
    WEAPON_ID_VIPERS =              2,
    WEAPON_ID_MAGMA_CANNON =        3,
    WEAPON_ID_ARBITER =             4,
    WEAPON_ID_FUSION_RIFLE =        5,
    WEAPON_ID_MINE_LAUNCHER =       6,
    WEAPON_ID_B6 =                  7,
    WEAPON_ID_OMNI_SHIELD =         8,
    WEAPON_ID_SWINGSHOT =           14,
    WEAPON_ID_FLAIL =               15
};

/*
 * NAME :		WEAPON_SLOT_INDEX
 * 
 * DESCRIPTION :
 * 			Defines the equip slot index for each weapon.
 * 
 * NOTES :
 *          
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
enum WEAPON_SLOT_INDEX
{
    WEAPON_SLOT_WRENCH =              0,
    WEAPON_SLOT_VIPERS =              1,
    WEAPON_SLOT_MAGMA_CANNON =        2,
    WEAPON_SLOT_ARBITER =             3,
    WEAPON_SLOT_FUSION_RIFLE =        4,
    WEAPON_SLOT_MINE_LAUNCHER =       5,
    WEAPON_SLOT_B6 =                  6,
    WEAPON_SLOT_FLAIL =               7,
    WEAPON_SLOT_OMNI_SHIELD =         8
};

/*
 * NAME :		ALPHA_MOD_IDS
 * 
 * DESCRIPTION :
 * 			Defines the value for each alpha mod.
 * 
 * NOTES :
 *          
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
enum ALPHA_MOD_IDS
{
    ALPHA_MOD_EMPTY =                   0,
    ALPHA_MOD_SPEED =                   1,
    ALPHA_MOD_AMMO =                    2,
    ALPHA_MOD_AIMING =                  3,
    ALPHA_MOD_IMPACT =                  4,
    ALPHA_MOD_AREA =                    5,
    ALPHA_MOD_XP =                      6,
    ALPHA_MOD_JACKPOT =                 7,
    ALPHA_MOD_NANOLEECH =               8,
};

/*
 * NAME :		OMEGA_MOD_IDS
 * 
 * DESCRIPTION :
 * 			Defines the value for each omega mod.
 * 
 * NOTES :
 *          
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
enum OMEGA_MOD_IDS
{
    OMEGA_MOD_EMPTY =                   0,
    OMEGA_MOD_NAPALM =                  1,
    OMEGA_MOD_TIME_BOMB =               2,
    OMEGA_MOD_FREEZE =                  3,
    OMEGA_MOD_MINI_BOMB =               4,
    OMEGA_MOD_MORPH =                   5,
    OMEGA_MOD_BRAINWASH =               6,
    OMEGA_MOD_ACID =                    7,
    OMEGA_MOD_SHOCK =                   8,
};

/*
 * NAME :		WeaponSlotToId
 * 
 * DESCRIPTION :
 * 			Converts a WEAPON_SLOT_INDEX to the respective WEAPON_IDS
 * 
 * NOTES :
 * 
 * ARGS : 
 *      slotId      :           Slot index.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int weaponSlotToId(int slotId);



#endif // _WEAPON_H_
