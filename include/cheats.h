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


/*
 * ------------------------------------------------
 * ----------- START DEADLOCKED OFFSETS -----------
 * ------------------------------------------------
 */
#if APPID == DL_APPID

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

#endif
/*
 * ------------------------------------------------
 * ------------ END DEADLOCKED OFFSETS ------------
 * ------------------------------------------------
 */

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
extern void cheatsApplyNoPacks(void);

/*
 *
 */
extern void cheatsApplyNoV2s(void);

/*
 *
 */
extern int cheatsDisableHealthboxes(void);


/*
 *
 */
extern void cheatsApplyMirrorWorld(int isOn);

/*
 *
 */
extern void cheatsApplyColorblindMode(u8 mode);

/*
 *
 */
extern void cheatsApplyWeather(u8 weatherId);


#endif // _CHEATS_H_
