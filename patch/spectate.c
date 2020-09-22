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

#include "string.h"
#include "player.h"
#include "game.h"
#include "gamesettings.h"
#include "stdio.h"
#include "pad.h"
#include "hud.h"


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
 * How sharp/snappy the camera position interpolation.
 * Higher is more sharp.
 */
const float CameraPositionSharpness = 50;

/*
 * How sharp/snappy the camera rotation interpolation.
 * Higher is more sharp.
 */
const float CameraRotationSharpness = 5;

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
    struct PlayerSpectateData * spectateData = SpectateData + currentPlayer->LocalPlayerIndex;
    if(!playerToSpectate)
        return;

    float cameraT = 1 - powf(MATH_E, -CameraRotationSharpness * MATH_DT);

    currentPlayer->CameraPitchMin = playerToSpectate->CameraPitchMin;
    currentPlayer->CameraPitchMax = playerToSpectate->CameraPitchMax;
    currentPlayer->CameraDistance = playerToSpectate->CameraDistance;

    // Interpolate camera rotation towards target player
    currentPlayer->CameraYaw.Value = spectateData->LastCameraYaw = lerpfAngle(spectateData->LastCameraYaw, playerToSpectate->CameraYaw.Value, cameraT);
    currentPlayer->CameraPitch.Value = spectateData->LastCameraPitch = lerpfAngle(spectateData->LastCameraPitch, playerToSpectate->CameraPitch.Value, cameraT);

    // Interpolate camera towards target player
    vector_lerp(spectateData->LastCameraPos, spectateData->LastCameraPos, playerToSpectate->CameraPos, 1 - powf(MATH_E, -CameraPositionSharpness * MATH_DT));
    vector_copy(currentPlayer->CameraPos, spectateData->LastCameraPos);


    // Something in this was crashing when swapping weapons
    //memcpy((((u8*)currentPlayer) + 0x1950), (((u8*)playerToSpectate) + 0x590), 0x20);

}

int findNextPlayerIndex(int currentPlayerIndex, int currentSpectateIndex, int direction)
{
    Player ** players = getPlayers();

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

void processSpectate(void) 
{
    GameSettings * gameSettings = getGameSettings();
	Player ** players = getPlayers();
    struct PlayerSpectateData * spectateData = 0;
    int i = 0;
    int direction = 0;
    int spectateIndex = 0;

    // First, we have to ensure we are in-game
	if (!gameSettings || !isInGame()) 
    {
        //memset(SpectateData, 0, sizeof(SpectateData));
        SpectateData->Enabled = 0;
		return;
    }

    // Loop through every player
    for (i = 0; i < GAME_MAX_PLAYERS; ++i)
	{
        if (!players[i])
		    continue;

		Player * player = players[i];

        // Next, we have to ensure the player is the local player and they are dead
	    if (isLocal(player)) 
        {
            // Grab player-specific spectate data
            spectateData = SpectateData + player->LocalPlayerIndex;
            spectateIndex = spectateData->Index;

            // 
            if (player->Health <= 0)
            {
                if (!spectateData->Enabled)
                {
                    // Show help message when player dies
                    if (!spectateData->HasShownEnterMsg) 
                    {
                        spectateData->HasShownEnterMsg = 1;
                        showHelpPopup(player->LocalPlayerIndex, "Press \x13 to enter spectate mode.", 5);
                    }

                    // When the player presses square and spectate isn't already enabled. Try to enable it.
                    if (playerPadGetButtonDown(player, PAD_SQUARE) > 0) 
                    {
                        // First check if there is a player to spectate
                        spectateIndex = findNextPlayerIndex(i, spectateIndex, 1);
                        if (spectateIndex >= 0)
                        {
                            spectateData->Enabled = 1;
                            spectateData->Index = spectateIndex;
                            getPlayerHUDFlags(player->LocalPlayerIndex)->Weapons = 0;
                            getPlayerHUDFlags(player->LocalPlayerIndex)->Healthbar = 0;
                            vector_copy(spectateData->LastCameraPos, players[spectateIndex]->CameraPos);
                        }
                    }
                }
                // Let the player exit spectate by pressing square
                else if (playerPadGetButtonDown(player, PAD_SQUARE) > 0)
                {
                    spectateData->Enabled = 0;
                    getPlayerHUDFlags(player->LocalPlayerIndex)->Weapons = 1;
                    getPlayerHUDFlags(player->LocalPlayerIndex)->Healthbar = 1;
                }
                // If the actively spectated player left find the next player
                else if (!players[spectateIndex])
                {
                    // First check if there is a player to spectate
                    spectateIndex = findNextPlayerIndex(i, spectateIndex, 1);
                    if (spectateIndex < 0)
                    {
                        spectateData->Enabled = 0;
                        getPlayerHUDFlags(player->LocalPlayerIndex)->Weapons = 1;
                        getPlayerHUDFlags(player->LocalPlayerIndex)->Healthbar = 1;
                    }

                    // Update spectate index
                    spectateData->Index = spectateIndex;
                }
                else
                {
                    // Show nav message
                    if (!spectateData->HasShownNavMsg) 
                    {
                        spectateData->HasShownNavMsg = 1;
                        showHelpPopup(player->LocalPlayerIndex, "Use \x14 and \x15 to navigate between players.", 10);
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
                    if (direction) {
                        spectateIndex = findNextPlayerIndex(i, spectateIndex, direction);
                    }

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
            else
            {
                spectateData->Enabled = 0;
                getPlayerHUDFlags(player->LocalPlayerIndex)->Weapons = 1;
                getPlayerHUDFlags(player->LocalPlayerIndex)->Healthbar = 1;
            }
        }
	}
}
