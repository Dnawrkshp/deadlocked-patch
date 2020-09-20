/***************************************************
 * FILENAME :		vector.h
 * 
 * DESCRIPTION :
 * 		
 * 
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _LIBDL_VECTOR_H_
#define _LIBDL_VECTOR_H_

//--------------------------------------------------------
typedef float VECTOR[4] __attribute__((__aligned__(16)));

//--------------------------------------------------------
void vector_copy(VECTOR output, VECTOR input0);
float vector_length(VECTOR input0);
void vector_lerp(VECTOR output, VECTOR input0, VECTOR input1, float t);
void vector_subtract(VECTOR output, VECTOR input0, VECTOR input1);
void vector_add(VECTOR output, VECTOR input0, VECTOR input1);
void vector_negate(VECTOR output, VECTOR input0);
void vector_scale(VECTOR output, VECTOR input0, float scalar);

#endif // _LIBDL_VECTOR_H_
