#include <libdl/pad.h>
#include <libdl/ui.h>
#include <libdl/graphics.h>
#include <libdl/stdio.h>
#include <libdl/net.h>
#include <libdl/color.h>
#include <libdl/gamesettings.h>
#include <libdl/spawnpoint.h>
#include <libdl/player.h>
#include <libdl/math.h>
#include <libdl/string.h>
#include <libdl/game.h>
#include <libdl/map.h>
#include "messageid.h"
#include "config.h"
#include "include/config.h"

#if MAPEDITOR

// config
extern PatchConfig_t config;

// game config
extern PatchGameConfig_t gameConfig;
extern PatchGameConfig_t gameConfigHostBackup;

int mapEditorState = 1;
int mapEditorRespawnState = 0;
int mapEditorLastHighlighted = 0;

u128 getVector(VECTOR v)
{
  return *(u128*)v;
}

int isSpawnPointHill(int id)
{
  int count = hillPointGetCount();
  int i;

  for (i = 0; i < count; ++i) {
    if (hillPointGetIndex(i) == id)
      return 1;
  }

  return 0;
}

void addSpawnPoint(SpawnPoint * sp)
{
  int * spTable = (int*)(*(u32*)0x0036DC58 + 0x1F8);
  int count = spawnPointGetCount();
  int i = count, j = 0;

  // check it's not a hill
  while (isSpawnPointHill(i))
    ++i;

  // set
  spawnPointSet(sp, i);

  // increment count
  (*(int*)0x00222824) = i + 1;

  // add reference to table
  for (j = 0; j < 64; ++j)
  {
    if (spTable[j] == -1)
      spTable[j] = i;
    if (spTable[j] == i)
      break;
  }
}

void removeSpawnPoint(int id)
{
  int * spTable = (int*)(*(u32*)0x0036DC58 + 0x1F8);
  int count = spawnPointGetCount();
  int i = id + 1;
  int l = id;

  // shift them down
  for (; i < count; ++i)
  {
    if (isSpawnPointHill(i))
      continue;

    spawnPointSet(spawnPointGet(i), l);
    l = i;
  }

  // decrement count
  count--;
  (*(int*)0x00222824) = count;

  // remove all references to >= count in spTable
  for (i = 0; i < 64; ++i)
    if (spTable[i] >= count)
      spTable[i] = -1;
}

void drawRing(MATRIX m3, VECTOR position, float scale0, float scale1, u32 color)
{
  // draw
  ((void (*)(void*, u128, float, float, int, int, int, int))0x00420c80)(
    m3,
    getVector(position),
    scale0,
    scale1,
    color,
    22,
    1,
    0
  );
}

void mapEditorDo(void)
{
  if (mapEditorState == 0)
    return;
  

  Player ** players = playerGetAll();
  Player * localPlayer = (Player*)0x347AA0;
  int count = spawnPointGetCount();
  int i = 0;
  int highlightedPoint = -1;
  VECTOR temp = {0,0,0,0};
  VECTOR temp2 = {0,0,0,0};

  // draw spawn points
  for (i = 0; i < count; ++i)
  {
    if (isSpawnPointHill(i))
      continue;

    SpawnPoint * sp = spawnPointGet(i);

    // check if player is near
    vector_copy(temp, (float*)&sp->M0[12]);
    temp[3] = 0;
    vector_subtract(temp, localPlayer->PlayerPosition, temp);
    u32 color = 0x80FFFFFF;
    if (highlightedPoint < 0 && vector_length(temp) < 4.0)
    {
      highlightedPoint = i;
      color = 0x8000FF00;
    }

    // draw
    vector_copy(temp, (float*)&sp->M0[12]);
    temp[2] += 1.0;
    drawRing(sp->M0, temp, 2.0, 1.0, color);

    // draw
    temp[0] += cosf(sp->M1[15]) * 2.5;
    temp[1] += sinf(sp->M1[15]) * 2.5;
    vector_subtract(temp2, temp, localPlayer->CameraPos);
    if (vector_innerproduct(temp2, localPlayer->CameraDir) > 0 && vector_length(temp2) < 100.0)
    {
      float scale = clamp(10.0 / vector_length(temp2), 0, 1);
      int x, y;
      gfxWorldSpaceToScreenSpace(temp, &x, &y);
      gfxScreenSpaceText(x, y, scale, scale, color, "+", -1, 4);
    }
  }
}

void onMapEditorGameUpdate(void)
{
  if (mapEditorState == 0)
    return;

  // hook
  if (*(u32*)0x004C4748 == 0x03E00008)
    *(u32*)0x004C4748 = 0x08000000 | ((u32)&mapEditorDo >> 2);;
  
  // 
  Player ** players = playerGetAll();
  Player * localPlayer = (Player*)0x347AA0;
  int count = spawnPointGetCount();
  int i = 0;
  int highlightedPoint = -1;
  VECTOR temp = {0,0,0,0};

  // always waiting for respawn
  if (mapEditorRespawnState)
    localPlayer->PlayerState = PLAYER_STATE_WAIT_FOR_RESURRECT;

  // find highlighted point
  for (i = 0; i < count; ++i)
  {
    if (isSpawnPointHill(i))
      continue;

    SpawnPoint * sp = spawnPointGet(i);

    // check if player is near
    vector_copy(temp, (float*)&sp->M0[12]);
    temp[3] = 0;
    vector_subtract(temp, localPlayer->PlayerPosition, temp);
    if (highlightedPoint < 0 && vector_length(temp) < 4.0)
    {
      highlightedPoint = i;
      break;
    }
  }

  // 
  if (mapEditorLastHighlighted != highlightedPoint && highlightedPoint >= 0)
  {
    mapEditorLastHighlighted = highlightedPoint;
    printf("Spawn Point #%d\n", mapEditorLastHighlighted);
  }

  // ignore inputs if configured
  if (*(u16*)((u32)localPlayer + 0x3BA) != 0)
    return;

  // handle inputs
  if (playerPadGetButtonDown(localPlayer, PAD_UP) > 0)
  {
    // add new spawn point
    SpawnPoint sp = {
      {
        1.0, 0, 0, 0,
        0, 1.0, 0, 0,
        0, 0, 1.0, 0,
        localPlayer->PlayerPosition[0], localPlayer->PlayerPosition[1], localPlayer->PlayerPosition[2], 0.01
      },
      { 
        1.0, 0, 0, 0,
        0, 1.0, 0, 0,
        0, 0, 1.0, 0,
        0, 0, 0, localPlayer->PlayerYaw 
      }
    };
    addSpawnPoint(&sp);
  }
  else if (playerPadGetButtonDown(localPlayer, PAD_DOWN) > 0 && highlightedPoint >= 0)
  {
    // remove existing spawn point
    removeSpawnPoint(highlightedPoint);
  }
}

#endif
