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

#include "stdio.h"
#include "math.h"
#include "math3d.h"
#include "time.h"
#include "module.h"
#include "game.h"
#include "gamesettings.h"
#include "player.h"
#include "cheats.h"
#include "sha1.h"
#include "map.h"

typedef struct BezierPoint
{
	VECTOR HandleIn;
	VECTOR ControlPoint;
	VECTOR HandleOut;
} BezierPoint;

BezierPoint TrackPoints[] = {
	{
		{ 440, 200, 108, 0 },
		{ 440, 205, 108, 0 },
		{ 440, 210, 108, 0 }
	},
	{
		{ 440, 230, 120, 0 },
		{ 440, 260, 120, 0 },
		{ 440, 290, 120, 0 }
	},
	{
		{ 470, 230, 110, 0 },
		{ 500, 230, 110, 0 },
		{ 530, 230, 110, 0 }
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

Moby * spawnWithPVars(int mobyId)
{
	switch (mobyId)
	{
		default: return spawnMoby(mobyId, 0);
	}
}

Moby * spawn(int mobyId, VECTOR position, VECTOR rotation, float scale)
{
	Moby * sourceBox;

	// Spawn box so we know the correct model and collision pointers
	sourceBox = spawnWithPVars(mobyId);

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
	sourceBox->ExtraPropertiesPointer = 0;

	// For this model the vector here is copied to 0x80 in the moby
	// This fixes the occlusion bug
	sourceBox->AnimationPointer = StartUNK_80;

	// 
#if DEBUG
	printf("source: %08x\n", (u32)sourceBox);
#endif

	return sourceBox;
}

void print_vector(VECTOR v)
{
	printf("<%s%d.%05d> <%s%d.%05d> <%s%d.%05d> <%s%d.%05d>",
		(v[0] < 0 ? "-" : "+"), (int)fabsf(v[0]), ((int)(fabsf(v[0]) * 100000)) % 100000,
		(v[1] < 0 ? "-" : "+"), (int)fabsf(v[0]), ((int)(fabsf(v[1]) * 100000)) % 100000,
		(v[2] < 0 ? "-" : "+"), (int)fabsf(v[0]), ((int)(fabsf(v[2]) * 100000)) % 100000,
		(v[3] < 0 ? "-" : "+"), (int)fabsf(v[0]), ((int)(fabsf(v[3]) * 100000)) % 100000);
}

void spawnTrack(void)
{
	VECTOR euler = { 0, 0, 0, 0 };
	VECTOR pos;
	VECTOR tangent;
	VECTOR normal;
	VECTOR bitangent;
	VECTOR right = { 1, 0, 0, 0 };
	VECTOR forward = { 0, 1, 0, 0 };
	VECTOR up = { 0, 0, 1, 0 };
	VECTOR r0 = { 0, 0, 0, 1 };
	MATRIX bezierRotationMatrix, startRotationMatrix;
	float scale = 3;
	float t = 0;
	float step = 0.05;
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

		for (t = 0; t <= 1; t += step)
		{
			// Calculate bezier
			BezierGetPosition(pos, &TrackPoints[b], &TrackPoints[b+1], t);
			BezierGetTangent(tangent, &TrackPoints[b], &TrackPoints[b+1], t);
			BezierGetNormal(normal, &TrackPoints[b], &TrackPoints[b+1], t);
			vector_outerproduct(bitangent, normal, tangent);

			// Determine euler rotation
			matrix_fromrows(bezierRotationMatrix, normal, bitangent, tangent, r0);
			matrix_multiply(bezierRotationMatrix, startRotationMatrix, bezierRotationMatrix);
			matrix_toeuler(euler, bezierRotationMatrix);
			
			// spawn
			spawn(MOBY_ID_OTHER_PART_FOR_SARATHOS_BRIDGE, pos, euler, scale);
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
	VECTOR rot = {0,0,0,0};

	// 
	GameSettings * gameSettings = getGameSettings();

	//
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
	GameSettings * gameSettings = getGameSettings();
	Player ** players = getPlayers();
	int i;

	// Ensure in game
	if (!gameSettings || !isInGame())
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
