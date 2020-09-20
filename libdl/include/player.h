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

#include "math.h"
#include "weapon.h"
#include "moby.h"
#include "pad.h"
#include "team.h"

/*
 * Maximum health of player.
 */
#define PLAYER_MAX_HEALTH                   (50)

/*
 * NAME :		PlayerCameraType
 * 
 * DESCRIPTION :
 * 			Defines the three player camera types.
 *          These are used in the player struct under 'CameraType'
 * 
 * NOTES :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
enum PlayerCameraType
{
    CAMERA_TYPE_THIRD_PERSON,
    CAMERA_TYPE_LOCK_STRAFE,
    CAMERA_TYPE_FIRST_PERSON
};

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
    char UNK0[0xA0];                                                // 0x00

    float PlayerX;                                                  // 0xA0
    float PlayerY;                                                  // 0xA4
    float PlayerZ;                                                  // 0xA8
    
    char UNK1[0x0C];                                                // 0xAC

    float PlayerYaw;                                                // 0xB8

    char UNK2[0x240];                                               // 0xBC

    int Airwalk;                                                    // 0x2FC

    char UNK3[0x70];                                                // 0x300

    // This resets whenever you jump/cboot/land
    // Appears to count how long since you've entered a state
    // Setting it to 0 when cbooting gives infinite cbooting
    int TicksSinceStateChanged;                                     // 0x370
    
    // More state related ticks
    int StateTicks[5];                                              // 0x374

    char UNK4[0x28];                                                // 0x388


    int RespawnTimer;                                               // 0x3B0

    char UNK30[0x38];                                               // 0x3B4

    float WeaponCooldownTimer;                                      // 0x3EC

    char UNK5[0x18];                                                // 0x3F0

    short ShieldTimer;                                              // 0x408
    short QuadTimer;                                                // 0x40A
    int UNK6;                                                       // 0x40C
    short UNK7;                                                     // 0x410
    short FireTimer;                                                // 0x412
    short AcidTimer;                                                // 0x414
    short FreezeTimer;                                              // 0x416
    short UNK8;                                                     // 0x418
    short JuggyElectricityTimer;                                    // 0x41A
    short UNK9;                                                     // 0x41C
    short BlowupTimer;                                              // 0x41E

    char UNK10[0x3DC];                                              // 0x420

    // First instance of weapon held id
    // This one is zero for any item that isn't a gun
    // So wrench and swingshot are 0
    int WeaponHeldGun;                                              // 0x7FC

    char UNK11[0x1150];                                             // 0x800

    Vector4 CameraPos;                                              // 0x1950
    char UNK99[0x50];                                               // 0x1960

    CameraAngle CameraYaw;                                          // 0x19B0
    CameraAngle CameraPitch;                                        // 0x19D0
    
    char UNK12[0x20];                                               // 0x19F0

    char UNK13[0xC];                                                // 0x1A10

    // This is a pointer to the moby that the
    // player is looking/aiming at
    Moby * LookAtMoby;                                              // 0x1A1C

    char UNK14[0x04];                                               // 0x1A20

    float CameraPitchMin;                                           // 0x1A24
    float CameraPitchMax;                                           // 0x1A28

    Moby * SkinMoby;                                                // 0x1A2C

    float CameraDistance;                                           // 0x1A30

    char UNK15[0x68];                                               // 0x1A34

    int LocalPlayerIndex;                                           // 0x1A9C

    char UNK16[0x8];                                                // 0x1AA0

    int PlayerId;                                                   // 0x1AA8

    char UNK33[0x34];                                               // 0x1AAC

    int CameraType;                                                 // 0x1AE0

    char UNK17[0x814];                                              // 0x1AE4

    // When 1, the player is trying to shoot
    int IsShooting;                                                 // 0x22F8

    char UNK29[0x18];                                               // 0x22FC

    // When this value is set to 3,
    // The weapon held is changed to the weapon id
    // set in WeaponHeldId
    int WeaponSwitchState;                                          // 0x2314
    int WeaponSwitchLastWeaponHeldId;                               // 0x2318

    char UNK28[0x194];                                              // 0x231C

    int WeaponHeldId0;                                              // 0x24B0

    char UNK18[0x118];                                              // 0x24B4

    int PlayerState;                                                // 0x25CC

    char UNK19[0x94];                                               // 0x25D0

    int NerfPlayer;                                                 // 0x2664

    char UNK31[0xC];                                                // 0x2668

    int ChangeWeaponHeldId;                                         // 0x2674

    char UNK20[0x14];                                               // 0x2678

    int WeaponHeldId;                                               // 0x268C

    char UNK27[0x54];                                               // 0x2690

    u32 WeaponHeldDataPointer;                                      // 0x26E4

    char UNK21[0x738];                                              // 0x26E8
    
    float Health;                                                   // 0x2E20
    
    char UNK22[0xCC];                                               // 0x2EEC

    // Points to the player moby
    Moby * PlayerMoby;                                              // 0x2EF0

    char UNK23[0x0C];                                               // 0x2EFC

    PadButtonStatus * Paddata;                                      // 0x2F00

    char UNK24[0x10];                                               // 0x2F04

    int Team;                                                       // 0x2F14

    char UNK25[0x98];                                               // 0x2F18

    float DamageMultiplier;                                         // 0x2FB0

    char UNK26[0x0C];                                               // 0x2FB4

    float Speed;                                                    // 0x2FC0

} Player;


/*
 * NAME :		getPlayers
 * 
 * DESCRIPTION :
 * 			Returns a pointer to an array of Player object pointers.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
Player ** getPlayers(void);

/*
 * NAME :		changeWeapon
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
void changeWeapon(Player * player, int weaponId);

/*
 * NAME :		GetPlayerWeaponData
 * 
 * DESCRIPTION :
 * 			Gets a pointer to the given player's weapon data struct.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      playerId      :           Player index.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
PlayerWeaponData * getPlayerWeaponData(int playerId);

/*
 * NAME :		setLocalPlayerEquipslot
 * 
 * DESCRIPTION :
 * 			Set's the given local player's equip slot with the given weapon id.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      localPlayerId      :           Local player index.
 *      
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void setLocalPlayerEquipslot(int localPlayerId, int slot, int weaponId);

/*
 * NAME :		changeTeam
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
void changeTeam(Player * player, int teamId);

/*
 * NAME :		isLocal
 * 
 * DESCRIPTION :
 * 			Whether or not the given player is local or remote.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      player      :           Target player.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int isLocal(Player * player);

/*
 * NAME :		giveWeapon
 * 
 * DESCRIPTION :
 * 			Converts a WEAPON_SLOT_INDEX to the respective WEAPON_IDS
 * 
 * NOTES :
 * 
 * ARGS : 
 *      player      :           Target player.
 *      weaponId    :           ID of weapon to give.
 *      weaponLevel :           Level of weapon.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void giveWeapon(Player * player, int weaponId, int weaponLevel);

/*
 * NAME :		playerSetPosRot
 * 
 * DESCRIPTION :
 * 			Sets a given player's position and rotation.
 * 
 * NOTES :
 * 
 * ARGS : 
 *      player      :           Target player.
 *      p           :           Position.
 *      r           :           Rotation.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void playerSetPosRot(Player * player, Vector3 * p, Vector3 * r);

#endif // _PLAYER_H_
