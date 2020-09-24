#include <string.h>
#include "math.h"
#include "math3d.h"

  
/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Naomi Peori <naomi@peori.ca>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

//--------------------------------------------------------
void vector_apply(VECTOR output, VECTOR input0, MATRIX input1)
{
    asm __volatile__ (
#if __GNUC__ > 3
    "lqc2   $vf1, 0x00(%2)  \n"
    "lqc2   $vf2, 0x10(%2)  \n"
    "lqc2   $vf3, 0x20(%2)  \n"
    "lqc2   $vf4, 0x30(%2)  \n"
    "lqc2   $vf5, 0x00(%1)  \n"
    "vmulaw   $ACC, $vf4, $vf0  \n"
    "vmaddax    $ACC, $vf1, $vf5  \n"
    "vmadday    $ACC, $vf2, $vf5  \n"
    "vmaddz   $vf6, $vf3, $vf5  \n"
    "sqc2   $vf6, 0x00(%0)  \n"
#else
    "lqc2		vf1, 0x00(%2)	\n"
    "lqc2		vf2, 0x10(%2)	\n"
    "lqc2		vf3, 0x20(%2)	\n"
    "lqc2		vf4, 0x30(%2)	\n"
    "lqc2		vf5, 0x00(%1)	\n"
    "vmulaw		ACC, vf4, vf0	\n"
    "vmaddax		ACC, vf1, vf5	\n"
    "vmadday		ACC, vf2, vf5	\n"
    "vmaddz		vf6, vf3, vf5	\n"
    "sqc2		vf6, 0x00(%0)	\n"
#endif
    : : "r" (output), "r" (input0), "r" (input1)
    );
}

//--------------------------------------------------------
void vector_copy(VECTOR output, VECTOR input0)
{
    asm __volatile__ (
#if __GNUC__ > 3
    "lqc2   $vf1, 0x00(%1)  \n"
    "sqc2   $vf1, 0x00(%0)  \n"
#else
    "lqc2		vf1, 0x00(%1)	\n"
    "sqc2		vf1, 0x00(%0)	\n"
#endif
    : : "r" (output), "r" (input0)
  );
}

//--------------------------------------------------------
void vector_normalize(VECTOR output, VECTOR input0)
{
    asm __volatile__ (
#if __GNUC__ > 3
    "lqc2   $vf1, 0x00(%1)  \n"
    "vmul.xyz   $vf2, $vf1, $vf1  \n"
    "vmulax.w   $ACC, $vf0, $vf2  \n"
    "vmadday.w    $ACC, $vf0, $vf2  \n"
    "vmaddz.w   $vf2, $vf0, $vf2  \n"
    "vrsqrt   $Q, $vf0w, $vf2w  \n"
    "vsub.w   $vf1, $vf0, $vf0  \n"
    "vwaitq       \n"
    "vmulq.xyz    $vf1, $vf1, $Q  \n"
    "sqc2   $vf1, 0x00(%0)  \n"
#else
    "lqc2		vf1, 0x00(%1)	\n"
    "vmul.xyz		vf2, vf1, vf1	\n"
    "vmulax.w		ACC, vf0, vf2	\n"
    "vmadday.w		ACC, vf0, vf2	\n"
    "vmaddz.w		vf2, vf0, vf2	\n"
    "vrsqrt		Q, vf0w, vf2w	\n"
    "vsub.w		vf1, vf0, vf0	\n"
    "vwaitq				\n"
    "vmulq.xyz		vf1, vf1, Q	\n"
    "sqc2		vf1, 0x00(%0)	\n"
#endif
    : : "r" (output), "r" (input0)
    );
}

//--------------------------------------------------------
float vector_length(VECTOR input0)
{
    return sqrtf((input0[0] * input0[0]) + (input0[1] * input0[1]) + (input0[2] * input0[2]));
}

//--------------------------------------------------------
float vector_innerproduct(VECTOR input0, VECTOR input1)
{
    VECTOR work0, work1;

    // Normalize the first vector.
    float m0 = vector_length(input0);
    work0[0] = (input0[0] / m0);
    work0[1] = (input0[1] / m0);
    work0[2] = (input0[2] / m0);
    work0[3] = (input0[3] / m0);

    // Normalize the second vector.
    float m1 = vector_length(input1);
    work1[0] = (input1[0] / m1);
    work1[1] = (input1[1] / m1);
    work1[2] = (input1[2] / m1);
    work1[3] = (input1[3] / m1);


    // Return the inner product.
    return (work0[0] * work1[0]) + (work0[1] * work1[1]) + (work0[2] * work1[2]);
}


//--------------------------------------------------------
void vector_lerp(VECTOR output, VECTOR input0, VECTOR input1, float t)
{
    VECTOR timeVector;
    timeVector[0] = t;

    asm __volatile__ (
#if __GNUC__ > 3
   "lqc2        $vf1, 0x00(%1)                  \n"             // load a into vf1
   "lqc2        $vf2, 0x00(%2)                  \n"             // load b into vf2
   "lqc2        $vf3, 0x00(%3)                  \n"             // load t into vf3
   "vsub.xyz    $vf2, $vf2, $vf1                \n"             // store b-a in vf2
   "vmulx.xyz   $vf2, $vf2, $vf3x               \n"             // multiply b-a by t
   "vadd.xyz    $vf1, $vf2, $vf1                \n"             // add (b-a)*t to a
   "sqc2        $vf1, 0x00(%0)                  \n"             // store result in out
#else
   "lqc2        vf1, 0x00(%1)                   \n"             // load a into vf1
   "lqc2        vf2, 0x00(%2)                   \n"             // load b into vf2
   "lqc2        vf3, 0x00(%3)                   \n"             // load t into vf3
   "vsub.xyz    vf2, $vf2, $vf1                 \n"             // store b-a in vf2
   "vmulx.xyz   vf2, $vf2, $vf3x                \n"             // multiply b-a by t
   "vadd.xyz    vf1, $vf2, $vf1                 \n"             // add (b-a)*t to a
   "sqc2        vf1, 0x00(%0)                   \n"             // store result in out
#endif
   : : "r" (output), "r" (input0), "r" (input1), "r" (timeVector)
  );
}

/*
void vector_subtract(VECTOR output, VECTOR input0, VECTOR input1);
void vector_add(VECTOR output, VECTOR input0, VECTOR input1);
void vector_negate(VECTOR output, VECTOR input0);
void vector_scale(VECTOR output, VECTOR input0, float scalar);
*/


void matrix_copy(MATRIX output, MATRIX input0)
{
    asm __volatile__ (
#if __GNUC__ > 3
    "lqc2   $vf1, 0x00(%1)  \n"
    "lqc2   $vf2, 0x10(%1)  \n"
    "lqc2   $vf3, 0x20(%1)  \n"
    "lqc2   $vf4, 0x30(%1)  \n"
    "sqc2   $vf1, 0x00(%0)  \n"
    "sqc2   $vf2, 0x10(%0)  \n"
    "sqc2   $vf3, 0x20(%0)  \n"
    "sqc2   $vf4, 0x30(%0)  \n"
#else
    "lqc2		vf1, 0x00(%1)	\n"
    "lqc2		vf2, 0x10(%1)	\n"
    "lqc2		vf3, 0x20(%1)	\n"
    "lqc2		vf4, 0x30(%1)	\n"
    "sqc2		vf1, 0x00(%0)	\n"
    "sqc2		vf2, 0x10(%0)	\n"
    "sqc2		vf3, 0x20(%0)	\n"
    "sqc2		vf4, 0x30(%0)	\n"
#endif
    : : "r" (output), "r" (input0)
    );
}

void matrix_inverse(MATRIX output, MATRIX input0)
{
    MATRIX work;

    // Calculate the inverse of the matrix.
    matrix_transpose(work, input0);
    work[0x03] = 0.00f;
    work[0x07] = 0.00f;
    work[0x0B] = 0.00f;
    work[0x0C] = -(input0[0x0C] * work[0x00] + input0[0x0D] * work[0x04] + input0[0x0E] * work[0x08]);
    work[0x0D] = -(input0[0x0C] * work[0x01] + input0[0x0D] * work[0x05] + input0[0x0E] * work[0x09]);
    work[0x0E] = -(input0[0x0C] * work[0x02] + input0[0x0D] * work[0x06] + input0[0x0E] * work[0x0A]);
    work[0x0F] = 1.00f;

    // Output the result.
    matrix_copy(output, work);
}

void matrix_multiply(MATRIX output, MATRIX input0, MATRIX input1)
{
    asm __volatile__ (
#if __GNUC__ > 3
    "lqc2   $vf1, 0x00(%1)  \n"
    "lqc2   $vf2, 0x10(%1)  \n"
    "lqc2   $vf3, 0x20(%1)  \n"
    "lqc2   $vf4, 0x30(%1)  \n"
    "lqc2   $vf5, 0x00(%2)  \n"
    "lqc2   $vf6, 0x10(%2)  \n"
    "lqc2   $vf7, 0x20(%2)  \n"
    "lqc2   $vf8, 0x30(%2)  \n"
    "vmulax.xyzw    $ACC, $vf5, $vf1  \n"
    "vmadday.xyzw $ACC, $vf6, $vf1  \n"
    "vmaddaz.xyzw $ACC, $vf7, $vf1  \n"
    "vmaddw.xyzw    $vf1, $vf8, $vf1  \n"
    "vmulax.xyzw    $ACC, $vf5, $vf2  \n"
    "vmadday.xyzw $ACC, $vf6, $vf2  \n"
    "vmaddaz.xyzw $ACC, $vf7, $vf2  \n"
    "vmaddw.xyzw    $vf2, $vf8, $vf2  \n"
    "vmulax.xyzw    $ACC, $vf5, $vf3  \n"
    "vmadday.xyzw $ACC, $vf6, $vf3  \n"
    "vmaddaz.xyzw $ACC, $vf7, $vf3  \n"
    "vmaddw.xyzw    $vf3, $vf8, $vf3  \n"
    "vmulax.xyzw    $ACC, $vf5, $vf4  \n"
    "vmadday.xyzw $ACC, $vf6, $vf4  \n"
    "vmaddaz.xyzw $ACC, $vf7, $vf4  \n"
    "vmaddw.xyzw    $vf4, $vf8, $vf4  \n"
    "sqc2   $vf1, 0x00(%0)  \n"
    "sqc2   $vf2, 0x10(%0)  \n"
    "sqc2   $vf3, 0x20(%0)  \n"
    "sqc2   $vf4, 0x30(%0)  \n"
#else
    "lqc2		vf1, 0x00(%1)	\n"
    "lqc2		vf2, 0x10(%1)	\n"
    "lqc2		vf3, 0x20(%1)	\n"
    "lqc2		vf4, 0x30(%1)	\n"
    "lqc2		vf5, 0x00(%2)	\n"
    "lqc2		vf6, 0x10(%2)	\n"
    "lqc2		vf7, 0x20(%2)	\n"
    "lqc2		vf8, 0x30(%2)	\n"
    "vmulax.xyzw		ACC, vf5, vf1	\n"
    "vmadday.xyzw	ACC, vf6, vf1	\n"
    "vmaddaz.xyzw	ACC, vf7, vf1	\n"
    "vmaddw.xyzw		vf1, vf8, vf1	\n"
    "vmulax.xyzw		ACC, vf5, vf2	\n"
    "vmadday.xyzw	ACC, vf6, vf2	\n"
    "vmaddaz.xyzw	ACC, vf7, vf2	\n"
    "vmaddw.xyzw		vf2, vf8, vf2	\n"
    "vmulax.xyzw		ACC, vf5, vf3	\n"
    "vmadday.xyzw	ACC, vf6, vf3	\n"
    "vmaddaz.xyzw	ACC, vf7, vf3	\n"
    "vmaddw.xyzw		vf3, vf8, vf3	\n"
    "vmulax.xyzw		ACC, vf5, vf4	\n"
    "vmadday.xyzw	ACC, vf6, vf4	\n"
    "vmaddaz.xyzw	ACC, vf7, vf4	\n"
    "vmaddw.xyzw		vf4, vf8, vf4	\n"
    "sqc2		vf1, 0x00(%0)	\n"
    "sqc2		vf2, 0x10(%0)	\n"
    "sqc2		vf3, 0x20(%0)	\n"
    "sqc2		vf4, 0x30(%0)	\n"
#endif
    : : "r" (output), "r" (input0), "r" (input1)
    );
}

void matrix_rotate(MATRIX output, MATRIX input0, VECTOR input1)
{
    MATRIX work;

    // Apply the z-axis rotation.
    matrix_unit(work);
    work[0x00] =  cosf(input1[2]);
    work[0x01] =  sinf(input1[2]);
    work[0x04] = -sinf(input1[2]);
    work[0x05] =  cosf(input1[2]);
    matrix_multiply(output, input0, work);

    // Apply the y-axis rotation.
    matrix_unit(work);
    work[0x00] =  cosf(input1[1]);
    work[0x02] = -sinf(input1[1]);
    work[0x08] =  sinf(input1[1]);
    work[0x0A] =  cosf(input1[1]);
    matrix_multiply(output, output, work);

    // Apply the x-axis rotation.
    matrix_unit(work);
    work[0x05] =  cosf(input1[0]);
    work[0x06] =  sinf(input1[0]);
    work[0x09] = -sinf(input1[0]);
    work[0x0A] =  cosf(input1[0]);
    matrix_multiply(output, output, work);
}

void matrix_scale(MATRIX output, MATRIX input0, VECTOR input1)
{
    MATRIX work;

    // Apply the scaling.
    matrix_unit(work);
    work[0x00] = input1[0];
    work[0x05] = input1[1];
    work[0x0A] = input1[2];
    matrix_multiply(output, input0, work);
}

void matrix_translate(MATRIX output, MATRIX input0, VECTOR input1)
{
    MATRIX work;

    // Apply the translation.
    matrix_unit(work);
    work[0x0C] = input1[0];
    work[0x0D] = input1[1];
    work[0x0E] = input1[2];
    matrix_multiply(output, input0, work);
}

void matrix_transpose(MATRIX output, MATRIX input0)
{
    MATRIX work;

    // Transpose the matrix.
    work[0x00] = input0[0x00];
    work[0x01] = input0[0x04];
    work[0x02] = input0[0x08];
    work[0x03] = input0[0x0C];
    work[0x04] = input0[0x01];
    work[0x05] = input0[0x05];
    work[0x06] = input0[0x09];
    work[0x07] = input0[0x0D];
    work[0x08] = input0[0x02];
    work[0x09] = input0[0x06];
    work[0x0A] = input0[0x0A];
    work[0x0B] = input0[0x0E];
    work[0x0C] = input0[0x03];
    work[0x0D] = input0[0x07];
    work[0x0E] = input0[0x0B];
    work[0x0F] = input0[0x0F];

    // Output the result.
    matrix_copy(output, work);
}

void matrix_unit(MATRIX output)
{
    // Create a unit matrix.
    memset(output, 0, sizeof(MATRIX));
    output[0x00] = 1.00f;
    output[0x05] = 1.00f;
    output[0x0A] = 1.00f;
    output[0x0F] = 1.00f;
}

void create_local_world(MATRIX local_world, VECTOR translation, VECTOR rotation)
{
    // Create the local_world matrix.
    matrix_unit(local_world);
    matrix_rotate(local_world, local_world, rotation);
    matrix_translate(local_world, local_world, translation);
}

void create_world_view(MATRIX world_view, VECTOR translation, VECTOR rotation)
{
    VECTOR work0, work1;

    // Reverse the translation.
    work0[0] = -translation[0];
    work0[1] = -translation[1];
    work0[2] = -translation[2];
    work0[3] = translation[3];

    // Reverse the rotation.
    work1[0] = -rotation[0];
    work1[1] = -rotation[1];
    work1[2] = -rotation[2];
    work1[3] = rotation[3];

    // Create the world_view matrix.
    matrix_unit(world_view);
    matrix_translate(world_view, world_view, work0);
    matrix_rotate(world_view, world_view, work1);
}

void create_view_screen(MATRIX view_screen, float aspect, float left, float right, float bottom, float top, float near, float far)
{
    // Apply the aspect ratio adjustment.
    left = (left * aspect); right = (right * aspect);

    // Create the view_screen matrix.
    matrix_unit(view_screen);
    view_screen[0x00] = (2 * near) / (right - left);
    view_screen[0x05] = (2 * near) / (top - bottom);
    view_screen[0x08] = (right + left) / (right - left);
    view_screen[0x09] = (top + bottom) / (top - bottom);
    view_screen[0x0A] = (far + near) / (far - near);
    view_screen[0x0B] = -1.00f;
    view_screen[0x0E] = (2 * far * near) / (far - near);
    view_screen[0x0F] = 0.00f;
}

void create_local_screen(MATRIX local_screen, MATRIX local_world, MATRIX world_view, MATRIX view_screen)
{
    // Create the local_screen matrix.
    matrix_unit(local_screen);
    matrix_multiply(local_screen, local_screen, local_world);
    matrix_multiply(local_screen, local_screen, world_view);
    matrix_multiply(local_screen, local_screen, view_screen);
}
