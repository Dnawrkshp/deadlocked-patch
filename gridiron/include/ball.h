#ifndef _GRIDIRON_BALL_H_
#define _GRIDIRON_BALL_H_

#include <libdl/moby.h>
#include <libdl/math3d.h>

Moby * ballSpawn(VECTOR position);
void ballPickup(Moby * ball, Player * player);
void ballThrow(Moby * ball);
void ballUpdate(Moby * ball);

#endif // _GRIDIRON_BALL_H_
