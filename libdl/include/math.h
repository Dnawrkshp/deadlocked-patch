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

//--------------------------------------------------------
#define MATH_PI                         (3.141596)
#define MATH_TAU                        (MATH_PI * 2.0)
#define MATH_E                          (2.7182818)
#define MATH_DT                         (0.166666666)
#define MATH_RAD2DEG                    (180 / MATH_PI)
#define MATH_DEG2RAD                    (MATH_PI / 180)

//--------------------------------------------------------
float cosf(float theta);
float sinf(float theta);
float acosf(float v);
float asinf(float v);
float sqrtf(float f);
float powf(float base, float exp);
float fabsf(float f);
float lerpf(float a, float b, float t);
float lerpfAngle(float a, float b, float t);
float clampAngle(float theta);

#endif // _MATH_H_
