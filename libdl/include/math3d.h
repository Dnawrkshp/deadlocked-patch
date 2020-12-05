/***************************************************
 * FILENAME :		math3d.h
 * 
 * DESCRIPTION :
 * 		
 * NOTES :
 *          Many functions are pulled from the PS2SDK.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Naomi Peori <naomi@peori.ca>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _LIBDL_MATH3D_H_
#define _LIBDL_MATH3D_H_

//--------------------------------------------------------
typedef float VECTOR[4] __attribute__((__aligned__(16)));
typedef float MATRIX[16] __attribute__((__aligned__(16)));

//--------------------------------------------------------
void vector_apply(VECTOR output, VECTOR input0, MATRIX input1);
void vector_copy(VECTOR output, VECTOR input0);
void vector_normalize(VECTOR output, VECTOR input0);
float vector_innerproduct(VECTOR input0, VECTOR input1);
void vector_outerproduct(VECTOR output, VECTOR input0, VECTOR input1);
float vector_length(VECTOR input0);
void vector_lerp(VECTOR output, VECTOR input0, VECTOR input1, float t);
void vector_subtract(VECTOR output, VECTOR input0, VECTOR input1);
void vector_add(VECTOR output, VECTOR input0, VECTOR input1);
void vector_negate(VECTOR output, VECTOR input0);
void vector_scale(VECTOR output, VECTOR input0, float scalar);

void quaternion_fromeuler(VECTOR output, VECTOR input0);
void quaternion_toeuler(VECTOR output, VECTOR input0);
void quaternion_lookrotation(VECTOR output, VECTOR input0, VECTOR input1);
void quaternion_multiply(VECTOR output, VECTOR input0, VECTOR input1);
void quaternion_frommatrix(VECTOR output, MATRIX input0);

void matrix_fromquaternion(MATRIX output, VECTOR input0);
void matrix_toeuler(VECTOR output, MATRIX input0);
void matrix_fromrows(MATRIX output, VECTOR input0, VECTOR input1, VECTOR input2, VECTOR input3);
void matrix_copy(MATRIX output, MATRIX input0);
void matrix_inverse(MATRIX output, MATRIX input0);
void matrix_multiply(MATRIX output, MATRIX input0, MATRIX input1);
void matrix_rotate(MATRIX output, MATRIX input0, VECTOR input1);
void matrix_scale(MATRIX output, MATRIX input0, VECTOR input1);
void matrix_translate(MATRIX output, MATRIX input0, VECTOR input1);
void matrix_transpose(MATRIX output, MATRIX input0);
void matrix_unit(MATRIX output);
float matrix_determinant(MATRIX input0);

#endif // _LIBDL_MATH3D_H_
