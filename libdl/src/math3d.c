#include "string.h"
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
