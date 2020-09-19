/***************************************************
 * FILENAME :		math.h
 * 
 * DESCRIPTION :
 * 		Contains many math related function definitions found in Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _MATH_H_
#define _MATH_H_

/*
 *
 */
#define MATH_PI                         (3.14159)

/*
 *
 */
#define MATH_TAU                        (MATH_PI * 2.0)


#if APPID == DL_APPID

/*
 * 
 */
#define MATH_COSF_FADDR                 (0x00135878)

/*
 * 
 */
#define MATH_SQRTF_FADDR                (0x00136C90) 

#endif

/*
 * 
 */
typedef struct Vector3
{
    float X;
    float Y;
    float Z;
} Vector3;

/*
 *
 */
typedef struct Vector4
{
    float X;
    float Y;
    float Z;
    float W;
} Vector4;


extern float (* cosf)(float);
float sinf(float);
extern float (* sqrtf)(float);

#endif // _MATH_H_
