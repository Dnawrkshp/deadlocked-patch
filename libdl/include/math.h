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
#define MATH_PI                         (3.14159)
#define MATH_TAU                        (MATH_PI * 2.0)
#define MATH_E                          (2.7182818)
#define MATH_DT                         (0.166666666)


//--------------------------------------------------------
float cosf(float theta);
float sinf(float theta);
float sqrtf(float f);
float powf(float base, float exp);

#endif // _MATH_H_
