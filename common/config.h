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
} PatchGameConfig_t;

#endif // _PATCH_CONFIG_
