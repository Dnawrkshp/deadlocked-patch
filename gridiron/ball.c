/***************************************************
 * FILENAME :		ball.c
 * 
 * DESCRIPTION :
 * 		Manages ball update and spawning.
 * 
 * NOTES :
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#include <tamtypes.h>
#include <string.h>

#include <libdl/time.h>
#include <libdl/game.h>
#include <libdl/gamesettings.h>
#include <libdl/graphics.h>
#include <libdl/player.h>
#include <libdl/weapon.h>
#include <libdl/hud.h>
#include <libdl/cheats.h>
#include <libdl/sha1.h>
#include <libdl/dialog.h>
#include <libdl/team.h>
#include <libdl/stdio.h>
#include <libdl/ui.h>
#include <libdl/guber.h>
#include <libdl/color.h>
#include <libdl/radar.h>
#include <libdl/sound.h>
#include <libdl/net.h>
#include "module.h"
#include "messageid.h"
#include "include/ball.h"

#define DRAG_COEFF (0.1)
#define BOUNCINESS_COEFF (0.5)
#define THROW_COEFF (1.2)
#define LIFETIME (25)

typedef struct BallPVars
{
  VECTOR Velocity;
  VECTOR StartPosition;
  Player * Carrier;
  int DieTime;
} BallPVars_t;

VECTOR VECTOR_GRAVITY = { 0, 0, -0.005, 0 };
VECTOR VECTOR_ZERO = { 0, 0, 0, 0 };

int (*CollLine_Fix)(VECTOR from, VECTOR to, u64 a2, Moby * damageSource, u64 t0) = 0x004b7e50;
int (*CollWaterHeight)(VECTOR from, VECTOR to, u64 a2, Moby * damageSource, u64 t0) = 0x00503780;
u128 (*vector_reflect)(u128 in, u128 normal) = 0x005b9ae8;

Moby * ballSpawn(VECTOR position)
{
  static Moby * BallVisualRefMoby = NULL;

  // we need to make sure we have our reference moby to
  // copy the model, animation, and collision of
  if (!BallVisualRefMoby)
  {
    BallVisualRefMoby = mobySpawn(0x1b37, 0);
    if (!BallVisualRefMoby)
      return NULL;
  }

  // spawn ball
	Moby * ball = mobySpawn(MOBY_ID_BETA_BOX, sizeof(BallPVars_t));
	DPRINTF("spawning new ball moby: %08X\n", (u32)ball);
	if (ball)
	{
    BallPVars_t * pvars = (BallPVars_t*)ball->PropertiesPointer;
    vector_copy(pvars->StartPosition, position);
    pvars->DieTime = 0;

		vector_copy(ball->Position, position);
		ball->UNK_30 = 0xFF;
		ball->UNK_31 = 0x01;
		ball->RenderDistance = 0x00FF;
		ball->Opacity = 0x80;
		ball->UNK_20[0] = 1;
		ball->UNK_20[2] = 0x37;

		ball->Scale = (float)0.02;
		ball->UNK_38[0] = 2;
		ball->UNK_38[1] = 2;
		ball->UNK_34[0] = 0x10;
		ball->PrimaryColor = 0xFFFF4040;
		ball->UNK_A8 = &ballUpdate;

		// animation stuff
		memcpy(&ball->AnimationPointer, &BallVisualRefMoby->AnimationPointer, 0x20);
		ball->UNK_48 = 4;

		ball->ModelPointer = BallVisualRefMoby->ModelPointer;
		ball->CollisionPointer = BallVisualRefMoby->CollisionPointer;
	}

  return ball;
}

void ballThrow(Moby * ball)
{
  VECTOR temp;
  if (!ball)
    return;

  BallPVars_t * pvars = (BallPVars_t*)ball->PropertiesPointer;
  Player * carrier = pvars->Carrier;

  // 
  if (!carrier)
    return;

  // remove carrier
  carrier->HeldMoby = NULL;
  pvars->Carrier = NULL;
  pvars->DieTime = gameGetTime() + (TIME_SECOND * LIFETIME);

  // set velocity
  vector_copy(pvars->Velocity, carrier->Velocity);

  // generate lob velocity from forward vector and pitch
  vector_scale(temp, (float*)((u32)carrier + 0x1A60), (1.5 - carrier->CameraPitch.Value) * THROW_COEFF);
  if (temp[2] < 0.1)
    temp[2] = 0.1;
  vector_add(pvars->Velocity, pvars->Velocity, temp);
}

void ballPickup(Moby * ball, Player * player)
{
  if (!ball)
    return;

  BallPVars_t * pvars = (BallPVars_t*)ball->PropertiesPointer;
  
  // drop from existing carrier
  if (pvars->Carrier && pvars->Carrier->HeldMoby == ball)
    pvars->Carrier->HeldMoby = NULL;

  // set new carrier
  pvars->Carrier = player;
  if (player)
    player->HeldMoby = ball;
}

void ballUpdate(Moby * ball)
{
  VECTOR updatePos, toPos, hitNormal, direction;
  float * hitPos = (float*)0x0023f930;
  if (!ball)
    return;

  BallPVars_t * pvars = (BallPVars_t*)ball->PropertiesPointer;
  Player * carrier = pvars->Carrier;

  // 
  if (!carrier && pvars->DieTime && gameGetTime() > pvars->DieTime)
  {
    mobyDestroy(ball);
    return;
  }

  // detect drop
  if (carrier && carrier->HeldMoby != ball)
  {
    vector_copy(pvars->Velocity, carrier->Velocity);
    pvars->Carrier = NULL;
    carrier = NULL;
    pvars->DieTime = gameGetTime() + (TIME_SECOND * LIFETIME);
  }

  // if not held do physics
  if (!carrier)
  {
    // detect if fell under map
    if (ball->Position[2] < gameGetDeathHeight())
    {
      vector_copy(ball->Position, pvars->StartPosition);
      vector_copy(pvars->Velocity, VECTOR_ZERO);
      return;
    }

    // add gravity
    vector_add(pvars->Velocity, pvars->Velocity, VECTOR_GRAVITY);

    // determine next update position
    vector_add(updatePos, ball->Position, pvars->Velocity);

    // add radius of model to next position for raycast
    vector_normalize(direction, pvars->Velocity);
    vector_scale(toPos, direction, 0.5);
    vector_add(toPos, updatePos, toPos);

    // hit detect
    if (CollLine_Fix(ball->Position, toPos, 2, 0, 0) != 0)
    {
      // get collision type
      u8 cType = *(u8*)0x0023F91C;
      if (cType == 0x0B || cType == 0x01 || cType == 0x04 || cType == 0x05 || cType == 0x0D || cType == 0x03)
      {
        vector_copy(ball->Position, pvars->StartPosition);
        vector_copy(pvars->Velocity, VECTOR_ZERO);
      }
      else
      {
        // change velocity based on hit
        vector_normalize(hitNormal, (float*)0x0023f940);
        *(u128*)pvars->Velocity = vector_reflect(*(u128*)pvars->Velocity, *(u128*)hitNormal);

        float dot = vector_innerproduct(direction, hitNormal);
        float exp = powf((dot + 1) / 2, 0.2);
        vector_scale(pvars->Velocity, pvars->Velocity, (clamp(1 - fabsf(dot), 0, 1) + BOUNCINESS_COEFF) * (1 - DRAG_COEFF));
        //vector_scale(pvars->Velocity, pvars->Velocity, 0.9);

        // correct position based on hit
        //vector_copy()
        
        //vector_copy(ball->Position, updatePos);
      }
    }
    else
    {
      vector_copy(ball->Position, updatePos);
    }
  }
}
