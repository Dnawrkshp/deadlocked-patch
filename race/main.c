/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Race entrypoint and logic.
 * 
 * NOTES :
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#include <tamtypes.h>
#include <string.h>

#include <libdl/stdio.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/time.h>
#include <libdl/game.h>
#include <libdl/gamesettings.h>
#include <libdl/player.h>
#include <libdl/cheats.h>
#include <libdl/sha1.h>
#include <libdl/map.h>
#include "module.h"

typedef struct BezierPoint
{
	VECTOR HandleIn;
	VECTOR ControlPoint;
	VECTOR HandleOut;
} BezierPoint;

BezierPoint TrackPoints[] = {
	{
		{ 434.6114, 129.1436, 106.79, 0 },
		{ 434.6114, 189.1436, 106.79, 0 },
		{ 434.6114, 204.6036, 106.79, 0 }
	},
	{
		{ 425.9593, 203.5597, 117.5641, 0 },
		{ 435.4033, 218.8396, 122.49, 0 },
		{ 444.8473, 234.1195, 127.4159, 0 }
	},
	{
		{ 464.4416, 247.7817, 140.4637, 0 },
		{ 483.2513, 240.1076, 142.84, 0 },
		{ 502.0611, 232.4336, 145.2163, 0 }
	},
	{
		{ 519.8095, 240.8596, 140.2249, 0 },
		{ 522.7714, 243.9236, 142.84, 0 },
		{ 525.7332, 246.9877, 145.4551, 0 }
	},
	{
		{ 519.9028, 271.334, 105.5986, 0 },
		{ 524.0114, 329.6436, 116.99, 0 },
		{ 524.6643, 338.9109, 118.8005, 0 }
	},
	{
		{ 528.6346, 414.5925, 124.75, 0 },
		{ 509.3113, 435.4436, 130.29, 0 },
		{ 502.9966, 442.2577, 132.1005, 0 }
	},
	{
		{ 470.1397, 411.2581, 111.4728, 0 },
		{ 427.9113, 456.5436, 114.91, 0 },
		{ 383.4917, 504.1791, 118.5255, 0 }
	},
	{
		{ 328.7635, 452.2501, 133.19, 0 },
		{ 325.1114, 402.3836, 133.19, 0 },
		{ 323.1684, 375.8547, 133.19, 0 }
	},
	{
		{ 344.6641, 359.2715, 149.9785, 0 },
		{ 356.2114, 370.8436, 153.39, 0 },
		{ 368.2288, 382.8869, 156.9405, 0 }
	},
	{
		{ 333.8216, 405.2706, 170.65, 0 },
		{ 322.5714, 391.3736, 170.65, 0 },
		{ 302.594, 366.6964, 170.65, 0 }
	},
	{
		{ 301.1886, 319.9573, 208.39, 0 },
		{ 303.8113, 280.1436, 208.39, 0 },
		{ 307.0849, 230.4513, 208.39, 0 }
	},
	{
		{ 269.7726, 129.7062, 123.483, 0 },
		{ 340.5914, 127.3836, 113.62, 0 },
		{ 357.5388, 126.8278, 111.2597, 0 }
	},
	{
		{ 434.6114, 129.1436, 106.79, 0 },
		{ 434.6114, 189.1436, 106.79, 0 },
		{ 434.6114, 204.6036, 106.79, 0 }
	}
};

/*
 *
 */
int Initialized = 0;

/*
 *
 */
VECTOR StartUNK_80 = {
	0.00514222,
	-0.0396723,
	62013.9,
	62013.9
};

void BezierGetPosition(VECTOR out, BezierPoint * a, BezierPoint * b, float t)
{
	VECTOR temp;
	float iT = 1 - t;

	vector_scale(out, a->ControlPoint, powf(iT, 3));
	vector_scale(temp, a->HandleOut, 3 * t * powf(iT, 2));
	vector_add(out, out, temp);
	vector_scale(temp, b->HandleIn, 3 * powf(t, 2) * iT);
	vector_add(out, out, temp);
	vector_scale(temp, b->ControlPoint, powf(t, 3));
	vector_add(out, out, temp);
}

void BezierGetTangent(VECTOR out, BezierPoint * a, BezierPoint * b, float t)
{
	VECTOR temp;
	const float delta = 0.001;
	const float invDelta = 1 / delta;

	if (t >= 1)
	{
		BezierGetPosition(temp, a, b, t - delta);
		BezierGetPosition(out, a, b, t);
	}
	else
	{
		BezierGetPosition(temp, a, b, t);
		BezierGetPosition(out, a, b, t + delta);
	}

	vector_subtract(out, out, temp);
	vector_scale(out, out, invDelta);
	vector_normalize(out, out);
}

void BezierGetNormal(VECTOR out, BezierPoint * a, BezierPoint * b, float t)
{
	VECTOR tangent;
	VECTOR up = {0,0,1,0};
	VECTOR right = {-1,0,0,0};

	BezierGetTangent(tangent, a, b, t);

	vector_outerproduct(out, tangent, right);
	if (vector_length(out) == 0)
		vector_outerproduct(out, tangent, up);
		
	vector_normalize(out, out);
}

float BezierMove(float t, BezierPoint * a, BezierPoint * b, float distance)
{
	VECTOR startPos, curPos;
	const float step = 0.01;

	// Get start position
	BezierGetPosition(startPos, a, b, t);

	while (t < 1)
	{
		t += step;

		// Get new position
		BezierGetPosition(curPos, a, b, t);

		// Check distance
		vector_subtract(curPos, curPos, startPos);
		if (vector_length(curPos) >= distance)
			return t;
	}

	if (t > 1)
		return 1;

	return t;
}

Moby * spawn(int mobyId, VECTOR position, VECTOR rotation, float scale)
{
	Moby * sourceBox;

	// Spawn
	sourceBox = mobySpawn(mobyId, 0);

	// 
	position[3] = sourceBox->Position[3];
	vector_copy(sourceBox->Position, position);

	//
	vector_copy(sourceBox->Rotation, rotation);

	sourceBox->UNK_30 = 0xFF;
	sourceBox->UNK_31 = 0x01;
	sourceBox->RenderDistance = 0x0080;
	sourceBox->Opacity = 0x7E;
	sourceBox->UNK_20[0] = 1;

	sourceBox->UNK_B8 = 1;
	sourceBox->Scale = (float)0.11 * scale;
	sourceBox->UNK_38[0] = 2;
	sourceBox->UNK_38[1] = 2;
	sourceBox->GuberMoby = 0;

	// For this model the vector here is copied to 0x80 in the moby
	// This fixes the occlusion bug
	sourceBox->AnimationPointer = StartUNK_80;

	// 
#if DEBUG
	printf("source: %08x\n", (u32)sourceBox);
#endif

	return sourceBox;
}

void spawnTrack(void)
{
	VECTOR euler = { 0, 0, 0, 0 };
	VECTOR pos;
	VECTOR tangent;
	VECTOR normal;
	VECTOR bitangent;
	VECTOR r0 = { 0, 0, 0, 1 };
	MATRIX bezierRotationMatrix, startRotationMatrix;
	float scale = 3;
	float t = 0;
	float stepDistance = 1.1 * scale;
	int b = 0;
	int bezierCount = sizeof(TrackPoints) / sizeof(BezierPoint);

	// initialize object rotation matrix
	matrix_unit(startRotationMatrix);
	matrix_rotate(startRotationMatrix, startRotationMatrix, (VECTOR){ 0, 90 * MATH_DEG2RAD, 0, 0 });

	#if DEBUG
	printf("BEGIN TRACK SPAWN\n");
	#endif

	for (b = 0; b < (bezierCount-1); ++b)
	{
		#if DEBUG
		printf("TRACK %d to %d\n", b, b+1);
		#endif

		t = 0;
		while (1)
		{
			// Calculate bezier
			BezierGetPosition(pos, &TrackPoints[b], &TrackPoints[b+1], t);
			BezierGetTangent(tangent, &TrackPoints[b], &TrackPoints[b+1], t);
			BezierGetNormal(normal, &TrackPoints[b], &TrackPoints[b+1], t);
			vector_copy(normal, (VECTOR){ 0,0,1,0 });
			vector_outerproduct(bitangent, normal, tangent);

			// Determine euler rotation
			matrix_fromrows(bezierRotationMatrix, normal, bitangent, tangent, r0);
			matrix_multiply(bezierRotationMatrix, startRotationMatrix, bezierRotationMatrix);
			matrix_toeuler(euler, bezierRotationMatrix);
			
			// spawn
			spawn(MOBY_ID_OTHER_PART_FOR_SARATHOS_BRIDGE, pos, euler, scale);

			// Get next time
			// If same as last time then break
			float nextT = BezierMove(t, &TrackPoints[b], &TrackPoints[b+1], stepDistance);
			if (nextT == t)
				break;

			t = nextT;
		}
	}

	#if DEBUG
	printf("END TRACK SPAWN\n");
	#endif
}

/*
 * NAME :		initialize
 * 
 * DESCRIPTION :
 * 			Initializes the gamemode.
 * 
 * NOTES :
 * 			This is called only once at the start.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void initialize(void)
{
	// 
	GameSettings * gameSettings = gameGetSettings();

	//
	if (gameSettings->GameLevel == MAP_ID_SARATHOS)
		spawnTrack();

	Initialized = 1;
}

/*
 * NAME :		gameStart
 * 
 * DESCRIPTION :
 * 			Race game logic entrypoint.
 * 
 * NOTES :
 * 			This is called only when in game.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void gameStart(void)
{
	GameSettings * gameSettings = gameGetSettings();
	Player ** players = playerGetAll();
	int i;

	// Ensure in game
	if (!gameSettings || !gameIsIn())
		return;

	if (!Initialized)
		initialize();
}

/*
 * NAME :		lobbyStart
 * 
 * DESCRIPTION :
 * 			Race lobby logic entrypoint.
 * 
 * NOTES :
 * 			This is called only when in lobby.
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void lobbyStart(void)
{

}
