

#ifndef __DEADLOCKED_PATCH_CONFIG__
#define __DEADLOCKED_PATCH_CONFIG__

typedef struct PatchConfig
{
  char disableFramelimiter;
  char enableGamemodeAnnouncements;
  char enableSpectate;
  char enableSingleplayerMusic;
  char levelOfDetail;
  char enablePlayerStateSync;
} PatchConfig_t;

//
enum PlayerStateConditionType
{
	PLAYERSTATECONDITION_REMOTE_EQUALS,
	PLAYERSTATECONDITION_LOCAL_EQUALS,
	PLAYERSTATECONDITION_LOCAL_OR_REMOTE_EQUALS
};

typedef struct PlayerStateCondition
{
	enum PlayerStateConditionType Type;
	int TimeSince;
	int StateId;
} PlayerStateCondition_t;

#endif
