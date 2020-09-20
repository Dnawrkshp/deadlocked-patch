
#include "moby.h"
#include "cheats.h"
#include "game.h"

/*
 * Address of spawn pack function.
 * This will be patched so that it exits right away and no pack is spawned.
 */
#define CHEAT_NO_PACKS_START                        ((u32*)0x00414660)


#define CHEAT_NO_V2S_JAL                            (*(u32*)0x00622310)


#define CHEAT_MIRROR_WORLD                          (*(u8*)0x0021DE31)
#define CHEAT_HUD_COLOR                             (*(u8*)0x0021DE32)
#define CHEAT_SMF_WRENCH                            (*(u8*)0x0021DE35)
#define CHEAT_VEHICLE_MORPH_MOD                     (*(u8*)0x0021DE36)
#define CHEAT_SUPER_BLOOM                           (*(u8*)0x0021DE37)
#define CHEAT_COLORBLIND_MODE                       (*(u8*)0x0021DE38)
#define CHEAT_WEATHER_MODE                          (*(u8*)0x0021DE39)
#define CHEAT_GANGSTA_GUNS                          (*(u8*)0x0021DE3A)
#define CHEAT_KILL_QWARK                            (*(u8*)0x0021DE3B)
#define CHEAT_NINJA_RATCHET                         (*(u8*)0x0021DE3C)
#define CHEAT_REMOVE_HELMET                         (*(u8*)0x0021DE3D)

#define CHEAT_CACHED_MIRROR_WORLD                   (*(u8*)0x0021DE41)
#define CHEAT_CACHED_HUD_COLOR                      (*(u8*)0x0021DE42)
#define CHEAT_CACHED_SMF_WRENCH                     (*(u8*)0x0021DE45)
#define CHEAT_CACHED_VEHICLE_MORPH_MOD              (*(u8*)0x0021DE46)
#define CHEAT_CACHED_SUPER_BLOOM                    (*(u8*)0x0021DE47)
#define CHEAT_CACHED_COLORBLIND_MODE                (*(u8*)0x0021DE48)
#define CHEAT_CACHED_WEATHER_MODE                   (*(u8*)0x0021DE49)
#define CHEAT_CACHED_GANGSTA_GUNS                   (*(u8*)0x0021DE4A)
#define CHEAT_CACHED_KILL_QWARK                     (*(u8*)0x0021DE4B)
#define CHEAT_CACHED_NINJA_RATCHET                  (*(u8*)0x0021DE4C)
#define CHEAT_CACHED_REMOVE_HELMET                  (*(u8*)0x0021DE4D)

#define CHEAT_UPDATE_FUNC                           ((void(*)(int))0x004AA178)

#define CHEAT_WEATHER_MODE_REAL                     (*(int*)0x002202DC)


//--------------------------------------------------------
void cheatsApplyNoPacks(void)
{
    *CHEAT_NO_PACKS_START = 0x03E00008;
    *(CHEAT_NO_PACKS_START+1) = 0x00000000;
}

//--------------------------------------------------------
void cheatsApplyNoV2s(void)
{
    // Nop function call
    CHEAT_NO_V2S_JAL = 0;
}

//--------------------------------------------------------
int cheatsDisableHealthboxes(void)
{
    int count = 0;

    // Iterate through mobys and disable healthboxes
	Moby ** mobiesArray = getLoadedMobies();
	Moby * currentMoby;
	while ((currentMoby = *mobiesArray))
	{
		if (currentMoby->MobyId == MOBY_ID_HEALTH_BOX_MULT)
		{
			currentMoby->PositionX = 0;
			currentMoby->PositionY = 0;
			currentMoby->PositionZ = 0;

			if (currentMoby->PropertiesPointer)
			{
				void * subPtr = (void*)(*(u32*)(currentMoby->PropertiesPointer));
				if (subPtr)
				{
					Moby * orb = (Moby*)(*(u32*)(subPtr + 0x98));
					if (orb)
					{
						orb->PositionX = 0;
						orb->PositionY = 0;
						orb->PositionZ = 0;

						
						++count;
					}
				}
				// 
			}
		}

		++mobiesArray;
	}

    return count;
}

//--------------------------------------------------------
void cheatsApplyMirrorWorld(int isOn)
{
    // 
    if (CHEAT_CACHED_MIRROR_WORLD == isOn)
        return;

    // Apply
    CHEAT_MIRROR_WORLD = isOn ? 1 : 0;
    
    if (isInGame())
    {
        // Update
        CHEAT_UPDATE_FUNC(0);
    }
    else
    {
        // If not in game then just set cached value
        CHEAT_CACHED_MIRROR_WORLD = isOn ? 1 : 0;
    }
}

//--------------------------------------------------------
void cheatsApplyColorblindMode(u8 mode)
{
    CHEAT_COLORBLIND_MODE = mode;
}

//--------------------------------------------------------
void cheatsApplyWeather(u8 weatherId)
{
    CHEAT_WEATHER_MODE = weatherId;
    CHEAT_CACHED_WEATHER_MODE = weatherId;

    // 
    if (weatherId == WEATHER_OFF)
        return;

    // Apply
    CHEAT_WEATHER_MODE_REAL = -weatherId;
}
