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
} SpectateData[2];

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
    if(!playerToSpectate)
        return;

    currentPlayer->CameraYaw = playerToSpectate->CameraYaw;
    currentPlayer->CameraPitch = playerToSpectate->CameraPitch;
    currentPlayer->CameraPos = playerToSpectate->CameraPos;
    currentPlayer->CameraPitchMin = playerToSpectate->CameraPitchMin;
    currentPlayer->CameraPitchMax = playerToSpectate->CameraPitchMax;
    currentPlayer->CameraDistance = playerToSpectate->CameraDistance;

    // Something in this was crashing when swapping weapons
    // memcpy((((u8*)currentPlayer) + 0x18D0), (((u8*)playerToSpectate) + 0x18D0), 0x200);

}

int findNextPlayerIndex(int currentPlayerIndex, int currentSpectateIndex, int direction)
{
    Player ** players = getPlayers();

    int newIndex = currentSpectateIndex;

    do 
    {
    loop:
        newIndex += direction;

        if(newIndex < 0)
            newIndex = GAME_MAX_PLAYERS-1;
        if(newIndex >= GAME_MAX_PLAYERS)
            newIndex = 0;
        if(newIndex == currentPlayerIndex && newIndex == currentSpectateIndex)
            return -1;
        if (newIndex == currentSpectateIndex)
            return -1;
        if(newIndex == currentPlayerIndex)
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
    int survivor = getGameSurvivor();
    int i = 0;
    int direction = 0;
    int spectateIndex = 0;

    // First, we have to ensure we are in-game
	if (!gameSettings || !isInGame()) 
    {
        memset(SpectateData, 0, sizeof(SpectateData));
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

                    // When the player presses square, or survivor is on, and spectate isn't already enabled. Enable it.
                    if (player->Paddata->square_p || survivor) 
                    {
                        spectateData->Enabled = 1;
                        spectateData->Index = i+1;
                    }
                }
                else
                {
                    // Show nav message
                    if (!spectateData->HasShownNavMsg) 
                    {
                        spectateData->HasShownNavMsg = 1;
                        showHelpPopup(player->LocalPlayerIndex, "Press \x15 to spectate the next player. Press \x14 to spectate the previous player.", 10);
                    }
                    
                    // If the currently spectated player becomes null, we move forward to the next player            
                    if (!players[spectateIndex])
                        direction = 1;
                    // If the player is pressing R1, move forward
                    else if (player->Paddata->r1_p) 
                        direction = 1;
                    // If the player is pressing L1, move backward
                    else if (player->Paddata->l1_p)
                        direction = -1;

                    if (direction) 
                    {
                        spectateIndex = findNextPlayerIndex(i, spectateIndex, direction);
                        if (spectateIndex < 0) 
                        {
                            SpectateData->Enabled = 0;
                            continue;
                        }
                    }

                    Player * nextPlayer = players[spectateData->Index = spectateIndex];
                    if (nextPlayer)
                        spectate(player, nextPlayer);
                }
            } else
            {
                spectateData->Enabled = 0;
                spectateData->HasShownEnterMsg = 0;
                spectateData->HasShownNavMsg = 0;
            }
        }
	}


    
    // Next, we check if survivor is on or not
    // - If survivor is on, automatically trigger spectate
    // - If survivor is off, logic to hook in and show option to turn on spectate
    
    
    // Spectate logic
    // When spectate is triggered
    // - Get list of active players in-game
    // - Remove yourself from the list of players to spectate
    // - Select the first player in that list
    // Pressing L1/R1 will move to the previous/next player index
    // Respawning automatically disables spectate
}
