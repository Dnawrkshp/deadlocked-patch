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
float vector_length(VECTOR input0);
void vector_lerp(VECTOR output, VECTOR input0, VECTOR input1, float t);
void vector_subtract(VECTOR output, VECTOR input0, VECTOR input1);
void vector_add(VECTOR output, VECTOR input0, VECTOR input1);
void vector_negate(VECTOR output, VECTOR input0);
void vector_scale(VECTOR output, VECTOR input0, float scalar);

//--------------------------------------------------------
/** Copy a matrix. */
void matrix_copy(MATRIX output, MATRIX input0);

/** Calculate the inverse of a matrix. */
void matrix_inverse(MATRIX output, MATRIX input0);

/** Multiply two matrices together. */
void matrix_multiply(MATRIX output, MATRIX input0, MATRIX input1);

/** Create a rotation matrix and apply it to the specified input matrix. */
void matrix_rotate(MATRIX output, MATRIX input0, VECTOR input1);

/** Create a scaling matrix and apply it to the specified input matrix. */
void matrix_scale(MATRIX output, MATRIX input0, VECTOR input1);

/** Create a translation matrix and apply it to the specified input matrix. */
void matrix_translate(MATRIX output, MATRIX input0, VECTOR input1);

/** Transpose a matrix. */
void matrix_transpose(MATRIX output, MATRIX input0);

/** Create a unit matrix. */
void matrix_unit(MATRIX output);

/** Create a local_world matrix given a translation and rotation.
 * Commonly used to describe an object's position and orientation.
 */
void create_local_world(MATRIX local_world, VECTOR translation, VECTOR rotation);

/** Create a world_view matrix given a translation and rotation.
 * Commonly used to describe a camera's position and rotation.
 */
void create_world_view(MATRIX world_view, VECTOR translation, VECTOR rotation);

/** Create a view_screen matrix given an aspect and clipping plane values.
 * Functionally similar to the opengl function: glFrustum()
 */
void create_view_screen(MATRIX view_screen, float aspect, float left, float right, float bottom, float top, float near, float far);

/** Create a local_screen matrix given a local_world, world_view and view_screen matrix.
 * Commonly used with vector_apply() to transform vertices for rendering.
 */
void create_local_screen(MATRIX local_screen, MATRIX local_world, MATRIX world_view, MATRIX view_screen);

#endif // _LIBDL_MATH3D_H_
