/***************************************************
 * FILENAME :		hud.h
 * 
 * DESCRIPTION :
 * 		Contains hud specific offsets and structures for Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _HUD_H_
#define _HUD_H_

/*
 * NAME :		PlayerHUDFlags
 * 
 * DESCRIPTION :
 * 			Each bit represents a hud 
 * 
 * NOTES :
 * 
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
typedef union PlayerHUDFlags
{
    struct 
    {
        int Weapons : 1;
        int Healthbar : 1;
        int BoltCounter : 1;
        int DpadSelect : 1;
        int Minimap : 1;
        int : 1;
        int ConquestScoreboard : 1;
        int ConquestUpgradeSelect : 1;
        int : 1;
        int NormalScoreboard : 1;
        int Meter : 1;
    };
    int raw;
} PlayerHUDFlags;


/*
 * NAME :		getPlayerHUDFlags
 * 
 * DESCRIPTION :
 * 			Gets the respective local player's HUD flags object.
 * 
 * NOTES :
 * 
 * ARGS : 
 *          localPlayerIndex:           Local index.
 * 
 * RETURN :
 *          Will return NULL if the localPlayerIndex is invalid.
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
PlayerHUDFlags * getPlayerHUDFlags(int localPlayerIndex);

#endif // _HUD_H_
