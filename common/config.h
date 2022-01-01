#ifndef _PATCH_CONFIG_
#define _PATCH_CONFIG_

typedef struct PatchConfig
{
  char disableFramelimiter;
  char enableGamemodeAnnouncements;
  char enableSpectate;
  char enableSingleplayerMusic;
  char levelOfDetail;
  char enablePlayerStateSync;
  char enableAutoMaps;
  char enableFpsCounter;
  char disableCircleToHackerRay;
} PatchConfig_t;

typedef struct PatchGameConfig
{
  char customMapId;
  char customModeId;
  char grWeatherId;
  char grNoPacks;
  char grNoV2s;
  char grMirrorWorld;
  char grNoHealthBoxes;
  char grVampire;
  char grHalfTime;
  char grBetterHills;
  char grHealthBars;
  char grNoNames;
} PatchGameConfig_t;

enum CUSTOM_MAP_ID
{
  CUSTOM_MAP_NONE = 0,
  CUSTOM_MAP_ANNIHILATION_NATION,
  CUSTOM_MAP_BAKISI_ISLES,
  CUSTOM_MAP_BATTLEDOME_SP,
  CUSTOM_MAP_BLACKWATER_DOCKS,
  CUSTOM_MAP_CONTAINMENT_SUITE,
  CUSTOM_MAP_DARK_CATHEDRAL_INTERIOR,
  CUSTOM_MAP_DESERT_PRISON,
  CUSTOM_MAP_DUCK_HUNT,
  CUSTOM_MAP_GHOST_SHIP,
  CUSTOM_MAP_HOVEN_GORGE,
  CUSTOM_MAP_HOVERBIKE_RACE,
  CUSTOM_MAP_LAUNCH_SITE,
  CUSTOM_MAP_MARCADIA_PALACE,
  CUSTOM_MAP_METROPOLIS_MP,
  CUSTOM_MAP_MINING_FACILITY_SP,
  CUSTOM_MAP_SARATHOS_SP,
  CUSTOM_MAP_SHAAR_SP,
  CUSTOM_MAP_SPLEEF,
  CUSTOM_MAP_TORVAL_SP,
  CUSTOM_MAP_TYHRRANOSIS,

  // always at the end to indicate how many items there are
  CUSTOM_MAP_COUNT
};

enum CUSTOM_MODE_ID
{
  CUSTOM_MODE_NONE = 0,
  CUSTOM_MODE_GUN_GAME,
  CUSTOM_MODE_INFECTED,
  CUSTOM_MODE_INFINITE_CLIMBER,
  CUSTOM_MODE_SEARCH_AND_DESTROY,
  CUSTOM_MODE_1000_KILLS,
  
#if DEV
  CUSTOM_MODE_GRIDIRON,
  CUSTOM_MODE_TEAM_DEFENDER,
#endif

  // always at the end to indicate how many items there are
  CUSTOM_MODE_COUNT
};

#endif // _PATCH_CONFIG_
