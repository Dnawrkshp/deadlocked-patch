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


#include "appid.h"



#if APPID == DL_APPID

/*
 * ------------------------------------------------
 * ----------- START DEADLOCKED OFFSETS -----------
 * ------------------------------------------------
 */


/*
 * Player 1's hud.
 */
#define HUD_P1			((PlayerHUDFlags*)0x0030D8E4)

/*
 * Player 2's hud.
 */
#define HUD_P2			((PlayerHUDFlags*)0x0030D8F4)

/*
 * ------------------------------------------------
 * ------------ END DEADLOCKED OFFSETS ------------
 * ------------------------------------------------
 */

#endif


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



#endif // _HUD_H_
