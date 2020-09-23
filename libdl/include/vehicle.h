/***************************************************
 * FILENAME :		vehicle.h
 * 
 * DESCRIPTION :
 * 		Contains vehicle specific offsets and structures for Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _LIBDL_VEHICLE_H_
#define _LIBDL_VEHICLE_H_

#include "math.h"
#include "weapon.h"
#include "moby.h"
#include "pad.h"
#include "team.h"
#include "math3d.h"
#include "player.h"

/*
 * NAME :		Vehicle
 * 
 * DESCRIPTION :
 * 			Contains the vehicle data.
 * 
 * NOTES :
 *          This is very large and documentation is incomplete.
 *          Much of the padding is to skip over area's not yet understood.
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
typedef struct Vehicle
{
    char UNK5[0xA0];                                                // 0x0000

    const VECTOR Position;                                          // 0x00A0

    char UNK0[0x174];                                               // 0x00B0

    const float VehiclePitch;                                       // 0x0224
    const float VehicleYaw;                                         // 0x0228

    char UNK3[0x18];                                                // 0x022C

    const float PassengerPitch;                                     // 0x0244
    const float PassengerYaw;                                       // 0x0248

    char UNK2[0x64];                                                // 0x024C

    Moby * VehicleMoby;                                             // 0x02B0

    char UNK1[0x04];                                                // 0x02B4

    struct Player * DriverPlayer;                                   // 0x02B8
    struct Player * PassengerPlayer;                                // 0x02BC

} Vehicle;


#endif // _LIBDL_VEHICLE_H_
