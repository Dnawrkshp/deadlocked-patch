/***************************************************
 * FILENAME :		spectate.c
 * 
 * DESCRIPTION :
 * 		Spectate mode entry point and logic.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Connor "Badger41" Williams
 */

#include <tamtypes.h>

#include <libdl/string.h>
#include <libdl/player.h>
#include <libdl/game.h>
#include <libdl/gamesettings.h>
#include <libdl/stdio.h>
#include <libdl/pad.h>
#include <libdl/hud.h>
#include <libdl/ui.h>


/*
 * How sharp/snappy the camera position interpolation.
 * Higher is more sharp.
 */
const float CAMERA_POSITION_SHARPNESS = 50;

/*
 * How sharp/snappy the camera rotation interpolation.
 * Higher is more sharp.
 */
const float CAMERA_ROTATION_SHARPNESS = 5;

/*
 * How sharp/snappy the camera rotation interpolation is in vehicle.
 * Higher is more sharp.
 */
const float VEHICLE_CAMERA_ROTATION_SHARPNESS = 0.3;

int Initialized = 0;

// This contains the spectate related info per local
struct PlayerSpectateData
{
    int Enabled;
    int Index;
    int HasShownEnterMsg;
    int HasShownNavMsg;
    VECTOR LastCameraPos;
    float LastCameraYaw;
    float LastCameraPitch;
} SpectateData[2];


/*
 * How far to spectate from a vehicle
 */
const float VEHICLE_DISTANCE[] =
{
    2,                  // hoverbike
    0,                  // hoverbike passenger
    3,                  // puma
    0,                  // puma passenger
    2,                  // landstalker
    0,                  // landstalker passenger
    4,                  // hovership
    0                   // hovership passenger
};

/*
 * How much elevation to add the camera position when spectate target is in vehicle.
 */
const float VEHICLE_ELEVATION[] =
{
    2,                  // hoverbike
    0,                  // hoverbike passenger
    3,                  // puma
    3,                  // puma passenger
    2.5,                // landstalker
    3,                  // landstalker passenger
    2,                  // hovership
    3                   // hovership passenger
};

/*
 * NAME :		enableSpectate
 * 
 * DESCRIPTION :
 * 			
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void enableSpectate(Player * player, struct PlayerSpectateData * data)
{
    // Fixes void fall bug
    *((u8*)0x00171DE0 + player->PlayerId) = 1;
    *(u32*)0x004DB88C = 0;

    data->Enabled = 1;
    hudGetPlayerFlags(player->LocalPlayerIndex)->Weapons = 0;
    hudGetPlayerFlags(player->LocalPlayerIndex)->Healthbar = 0;
}

/*
 * NAME :		disableSpectate
 * 
 * DESCRIPTION :
 * 			
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void disableSpectate(Player * player, struct PlayerSpectateData * data)
{
    *((u8*)0x00171DE0 + player->PlayerId) = 0;
    *(u32*)0x004DB88C = 0xA48200E0;

    data->Enabled = 0;
    hudGetPlayerFlags(player->LocalPlayerIndex)->Weapons = 1;
    hudGetPlayerFlags(player->LocalPlayerIndex)->Healthbar = 1;
}

/*
 * NAME :		spectate
 * 
 * DESCRIPTION :
 * 			Spectates the given player.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 		playerId:	Player index
 * 
 * RETURN :
 * 
 * AUTHOR :			Connor "Badger41" Williams
 */
void spectate(Player * currentPlayer, Player * playerToSpectate)
{
    float cameraT;
    struct PlayerSpectateData * spectateData = SpectateData + currentPlayer->LocalPlayerIndex;
    if(!playerToSpectate)
        return;

    // Fix void fall spectate bug
    currentPlayer->CameraType2 = 2;

    currentPlayer->CameraPitchMin = playerToSpectate->CameraPitchMin;
    currentPlayer->CameraPitchMax = playerToSpectate->CameraPitchMax;
    currentPlayer->CameraDistance = -6;

    if (playerToSpectate->Vehicle)
    {
        Moby * vehicleMoby = playerToSpectate->Vehicle->VehicleMoby;
        int isPassenger = playerToSpectate->Vehicle->PassengerPlayer == playerToSpectate;
        cameraT = 1 - powf(MATH_E, -(isPassenger ? CAMERA_ROTATION_SHARPNESS : VEHICLE_CAMERA_ROTATION_SHARPNESS) * MATH_DT);

        // Grab rotation
        float yaw = playerToSpectate->Vehicle->VehicleYaw;
        float pitch = 0.08;
        float distance = 0;
        float elevation = 0;
        
        // Determine distance and elevation by vehicle id
        switch (vehicleMoby->MobyId)
        {
            case MOBY_ID_HOVERBIKE:
            {
                yaw = playerToSpectate->PlayerYaw;
                distance = VEHICLE_DISTANCE[0 + isPassenger];
                elevation = VEHICLE_ELEVATION[0 + isPassenger];
                break;
            }
            case MOBY_ID_PUMA:
            {
                distance = VEHICLE_DISTANCE[2 + isPassenger];
                elevation = VEHICLE_ELEVATION[2 + isPassenger];

                if (isPassenger)
                {
                    yaw = playerToSpectate->Vehicle->PassengerYaw;
                    pitch = playerToSpectate->Vehicle->PassengerPitch + (float)0.08;
                }
                break;
            }
            case MOBY_ID_LANDSTALKER:
            {
                distance = VEHICLE_DISTANCE[4 + isPassenger];
                elevation = VEHICLE_ELEVATION[4 + isPassenger];

                if (isPassenger)
                {
                    yaw = playerToSpectate->Vehicle->PassengerYaw;
                    pitch = playerToSpectate->Vehicle->PassengerPitch;
                }
                break;
            }
            case MOBY_ID_HOVERSHIP:
            {
                pitch = playerToSpectate->Vehicle->VehiclePitch;
                yaw = playerToSpectate->Vehicle->VehicleYaw;
                distance = VEHICLE_DISTANCE[6 + isPassenger];
                elevation = VEHICLE_ELEVATION[6 + isPassenger];

                // Get passenger camera rotation -- only works with local
                if (isPassenger && playerIsLocal(playerToSpectate))
                {
                    float * props = (float*)((*(u32*)((u32)vehicleMoby->PropertiesPointer + 0x10)) + 0x180);
                    pitch = props[1];
                    yaw = props[0];
                }
                break;
            }
        }

        // Interpolate camera rotation towards target player
        currentPlayer->CameraYaw.Value = spectateData->LastCameraYaw = lerpfAngle(spectateData->LastCameraYaw, yaw, cameraT);
        currentPlayer->CameraPitch.Value = spectateData->LastCameraPitch = lerpfAngle(spectateData->LastCameraPitch, pitch, cameraT);
        
        // Generate target based off distance and elevation
        VECTOR target;
        vector_copy(target, playerToSpectate->Vehicle->VehicleMoby->Position);
        target[0] -= cosf(spectateData->LastCameraYaw) * distance;
        target[1] -= sinf(spectateData->LastCameraYaw) * distance;
        target[2] += (sinf(spectateData->LastCameraPitch) * distance) + elevation;

        // Interpolate camera towards target player
        vector_lerp(spectateData->LastCameraPos, spectateData->LastCameraPos, target, 1 - powf(MATH_E, -CAMERA_POSITION_SHARPNESS * MATH_DT));
        vector_copy(currentPlayer->CameraPos, spectateData->LastCameraPos);
    }
    else
    {
        cameraT = 1 - powf(MATH_E, -CAMERA_ROTATION_SHARPNESS * MATH_DT);

        // Interpolate camera rotation towards target player
        currentPlayer->CameraYaw.Value = spectateData->LastCameraYaw = lerpfAngle(spectateData->LastCameraYaw, playerToSpectate->CameraYaw.Value, cameraT);
        currentPlayer->CameraPitch.Value = spectateData->LastCameraPitch = lerpfAngle(spectateData->LastCameraPitch, playerToSpectate->CameraPitch.Value, cameraT);

        // Interpolate camera towards target player
        vector_lerp(spectateData->LastCameraPos, spectateData->LastCameraPos, playerToSpectate->CameraPos, 1 - powf(MATH_E, -CAMERA_POSITION_SHARPNESS * MATH_DT));
        vector_copy(currentPlayer->CameraPos, spectateData->LastCameraPos);
    }
}

int findNextPlayerIndex(int currentPlayerIndex, int currentSpectateIndex, int direction)
{
    Player ** players = playerGetAll();

    int newIndex = currentSpectateIndex;

    do 
    {
    loop:
        newIndex += direction;

        if (newIndex < 0)
            newIndex = GAME_MAX_PLAYERS-1;
        else if (newIndex >= GAME_MAX_PLAYERS)
            newIndex = 0;
        // Fail if the only player is us
        if(newIndex == currentPlayerIndex && newIndex == currentSpectateIndex)
            return -1;
        // prevent infinite loop
        if (newIndex == currentSpectateIndex)
            return players[currentSpectateIndex] ? currentSpectateIndex : -1;
        // skip self
        if (newIndex == currentPlayerIndex)
            goto loop;
    }
    while(!players[newIndex]);
    
    return newIndex;
}

void initialize(void)
{
    memset(SpectateData, 0, sizeof(SpectateData));
    Initialized = 1;
}

void processSpectate(void) 
{
    GameSettings * gameSettings = gameGetSettings();
	Player ** players = playerGetAll();
    struct PlayerSpectateData * spectateData = 0;
    int i = 0;
    int direction = 0;
    int spectateIndex = 0;

    // First, we have to ensure we are in-game
	if (!gameSettings || !gameIsIn()) 
    {
        SpectateData->Enabled = 0;
        Initialized = 0;
		return;
    }

    if (Initialized != 1)
        initialize();

    // Loop through every player
    for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
        if (!players[i])
		    continue;

		Player * player = players[i];

        // Next, we have to ensure the player is the local player and they are dead
	    if (playerIsLocal(player)) 
        {
            // Grab player-specific spectate data
            spectateData = SpectateData + player->LocalPlayerIndex;
            spectateIndex = spectateData->Index;

            // If dead
            if ((player->PlayerState & 0xFFFF) == 0x99)
            {
                if (!spectateData->Enabled)
                {
                    // Show help message when player dies
                    if (!spectateData->HasShownEnterMsg) 
                    {
                        spectateData->HasShownEnterMsg = 1;
                        uiShowHelpPopup(player->LocalPlayerIndex, "Press \x13 to enter spectate mode.", 5 * 30);
                    }

                    // When the player presses square and spectate isn't already enabled. Try to enable it.
                    if (playerPadGetButtonDown(player, PAD_SQUARE) > 0) 
                    {
                        // First check if there is a player to spectate
                        spectateIndex = findNextPlayerIndex(i, spectateIndex, 1);
                        if (spectateIndex >= 0)
                        {
                            enableSpectate(player, spectateData);
                            spectateData->Index = spectateIndex;
                            vector_copy(spectateData->LastCameraPos, players[spectateIndex]->CameraPos);
                        }
                    }
                }
                // Let the player exit spectate by pressing square
                else if (playerPadGetButtonDown(player, PAD_SQUARE) > 0)
                {
                    disableSpectate(player, spectateData);
                }
                // If the actively spectated player left find the next player
                else if (!players[spectateIndex])
                {
                    // First check if there is a player to spectate
                    spectateIndex = findNextPlayerIndex(i, spectateIndex, 1);
                    if (spectateIndex < 0)
                        disableSpectate(player, spectateData);

                    // Update spectate index
                    spectateData->Index = spectateIndex;
                }
                else
                {
                    // Show nav message
                    if (!spectateData->HasShownNavMsg) 
                    {
                        spectateData->HasShownNavMsg = 1;
                        uiShowHelpPopup(player->LocalPlayerIndex, "Use \x14 and \x15 to navigate between players.", 10 * 30);
                    }
                    
                    // If the currently spectated player becomes null, we move forward to the next player            
                    if (!players[spectateIndex])
                        direction = 1;
                    // If the player is pressing R1, move forward
                    else if (playerPadGetButtonDown(player, PAD_R1) > 0) 
                        direction = 1;
                    // If the player is pressing L1, move backward
                    else if (playerPadGetButtonDown(player, PAD_L1) > 0)
                        direction = -1;

                    // 
                    if (direction)
                        spectateIndex = findNextPlayerIndex(i, spectateIndex, direction);

                    if (spectateIndex >= 0)
                    {
                        Player * nextPlayer = players[spectateIndex];
                        if (nextPlayer)
                        {
                            // Update last camera position to new target
                            // This snaps the camera to the new target instead of lerping
                            if (spectateIndex != spectateData->Index)
                                vector_copy(spectateData->LastCameraPos, nextPlayer->CameraPos);

                            spectate(player, nextPlayer);
                        }
                    }

                    // Finally update stored index value
                    spectateData->Index = spectateIndex;
                }
            }
            else if (spectateData->Enabled)
            {
                disableSpectate(player, spectateData);
            }
        }
	}
}
