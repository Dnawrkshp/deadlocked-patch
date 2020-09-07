/***************************************************
 * FILENAME :		player.h
 * 
 * DESCRIPTION :
 * 		Contains player specific offsets and structures for Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _PLAYER_H_
#define _PLAYER_H_


#include "appid.h"
#include "weapon.h"
#include "moby.h"
#include "pad.h"
#include "team.h"



#if APPID == DL_APPID

/*
 * ------------------------------------------------
 * ----------- START DEADLOCKED OFFSETS -----------
 * ------------------------------------------------
 */


/*
 * 
 */
#define PLAYER_STRUCT_ARRAY                 ((Player**)0x00344C38)

/*
 * Local player 1 dme player index.
 */
#define PLAYER_1_ID                         (*(u32*)0x0017218C)

/*
 * Local player 2 dme player index.
 */
#define PLAYER_2_ID                         (*(u32*)0x001B6ED8)

/*
 *
 */
#define PLAYER_GIVEWEP_FUNCADD              (0x00628018)

/*
 * ------------------------------------------------
 * ------------ END DEADLOCKED OFFSETS ------------
 * ------------------------------------------------
 */

#endif


/*
 * NAME :		CameraAngle
 * 
 * DESCRIPTION :
 * 			Contains camera angle settings and information.
 * 
 * NOTES :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
typedef struct CameraAngle
{
    float Value;
    float Velocity;
    float MaxVelocity;
    float UNK0;
    float Acceleration;
    float UNK1[3];
} CameraAngle;

/*
 * NAME :		Player
 * 
 * DESCRIPTION :
 * 			Contains the player data.
 * 
 * NOTES :
 *          This is very large and documentation is incomplete.
 *          Much of the padding is to skip over area's not yet understood.
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
typedef struct Player
{
    char UNK0[0xA0];

    float PlayerX;
    float PlayerY;
    float PlayerZ;
    
    char UNK1[0x0C];

    float PlayerYaw;

    char UNK2[0x240];

    int Airwalk;

    char UNK3[0x70];

    // This resets whenever you jump/cboot/land
    // Appears to count how long since you've entered a state
    // Setting it to 0 when cbooting gives infinite cbooting
    int TicksSinceStateChanged;
    
    // More state related ticks
    int StateTicks[5];

    char UNK4[0x28];


    int RespawnTimer;

    char UNK30[0x38];

    float WeaponCooldownTimer;

    char UNK5[0x18];

    short ShieldTimer;
    short QuadTimer;
    int UNK6;
    short UNK7;
    short FireTimer;
    short AcidTimer;
    short FreezeTimer;
    short UNK8;
    short JuggyElectricityTimer;
    short UNK9;
    short BlowupTimer;

    char UNK10[0x3DC];

    // First instance of weapon held id
    // This one is zero for any item that isn't a gun
    // So wrench and swingshot are 0
    int WeaponHeldGun;

    char UNK11[0x11B0];

    CameraAngle CameraYaw;
    char UNK12[0x20];
    CameraAngle CameraPitch;

    char UNK13[0xC];

    // This is a pointer to the moby that the
    // player is looking/aiming at
    Moby * LookAtMoby;

    char UNK14[0x30];

    float CameraDistance;

    char UNK15[0x48];

    int LocalPlayerIndex;

    char UNK16[0x8];

    int PlayerId;

    char UNK17[0x84C];

    // When 1, the player is trying to shoot
    int IsShooting;

    char UNK29[0x18];

    // When this value is set to 3,
    // The weapon held is changed to the weapon id
    // set in WeaponHeldId
    int WeaponSwitchState;
    int WeaponSwitchLastWeaponHeldId;

    char UNK28[0x194];

    int WeaponHeldId0;

    char UNK18[0x118];

    int PlayerState;

    char UNK19[0x94];

    int IsDead;

    char UNK31[0xC];

    int ChangeWeaponHeldId;

    char UNK20[0x14];

    int WeaponHeldId;

    char UNK27[0x54];

    u32 WeaponHeldDataPointer;

    char UNK21[0x738];
    
    float Health;
    
    char UNK22[0xCC];

    // Points to the player moby
    Moby * PlayerMoby;

    char UNK23[0x0C];

    PadButtonStatus * Paddata;

    char UNK24[0x10];

    int Team;

    char UNK25[0x98];

    float DamageMultiplier;

    char UNK26[0x0C];

    float Speed;

} Player;

void ChangeWeapon(Player * player, int weaponId);
void ChangeTeam(Player * player, int teamId);
int IsLocal(Player * player);

/*
 * NAME :		GiveWeapon
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
void GiveWeapon(Player * player, int weaponId, int weaponLevel);


#endif // _PLAYER_H_
