/***************************************************
 * FILENAME :		cheats.h
 * 
 * DESCRIPTION :
 * 		Contains a miscellaneous collection of cheats for Ratchet: Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _CHEATS_H_
#define _CHEATS_H_

#include <tamtypes.h>

/*
 * Weather ids
 */
enum WeatherIds
{
    WEATHER_OFF =                       0,
    WEATHER_DUST_STORM =                1,
    WEATHER_HEAVY_SAND_STORM =          2,
    WEATHER_LIGHT_SNOW =                3,
    WEATHER_BLIZZARD =                  4,
    WEATHER_HEAVY_RAIN =                5,
    WEATHER_ALL_OFF =                   6,
    WEATHER_GREEN_MIST =                7,
    WEATHER_METEOR_LIGHTNING =          9,
    WEATHER_BLACK_HOLE =                10,
    WEATHER_LIGHT_RAIN_LIGHTNING =      11,
    WEATHER_SETTLING_SMOKE =            12,
    WEATHER_UPPER_ATMOSPHERE =          13,
    WEATHER_GHOST_STATION =             14,
    WEATHER_EMBOSSED =                  15,
    WEATHER_LIGHTNING_STORM =           16
};

/*
 * Colorblind modes.
 */
enum ColorblindModes
{
    COLORBLIND_OFF =        0,
    COLORBLIND_GRAYSCALE =  1,
    COLORBLIND_SEPIA =      2
};

/*
 *
 */
void cheatsApplyNoPacks(void);

/*
 *
 */
void cheatsApplyNoV2s(void);

/*
 *
 */
int cheatsDisableHealthboxes(void);


/*
 *
 */
void cheatsApplyMirrorWorld(int isOn);

/*
 *
 */
void cheatsApplyColorblindMode(u8 mode);

/*
 *
 */
void cheatsApplyWeather(u8 weatherId);


#endif // _CHEATS_H_
