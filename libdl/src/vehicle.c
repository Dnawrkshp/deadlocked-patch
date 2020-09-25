#include "player.h"
#include "vehicle.h"


/*
 * 
 */
#define VEHICLE_KICK_PLAYER_FUNC                        (0x00619140)

//--------------------------------------------------------
void VehicleRemovePlayer(Vehicle * vehicle, struct Player * player)
{
    ((void (*)(Vehicle *, struct Player *))VEHICLE_KICK_PLAYER_FUNC)(vehicle, player);
}
