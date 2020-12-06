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
void vector_outerproduct(VECTOR output, VECTOR input0, VECTOR input1)
{
    asm __volatile__ (
#if __GNUC__ > 3
    "lqc2		$vf1, 0x00(%1)	\n"
    "lqc2		$vf2, 0x00(%2)	\n"
    "vopmula.xyz		$ACC, $vf1, $vf2\n"
    "vopmsub.xyz		$vf2, $vf2, $vf1\n"
    "vsub.w		$vf2, $vf0, $vf0\n"
    "sqc2		$vf2, 0x00(%0)	\n"
#else
    "lqc2		vf1, 0x00(%1)	\n"
    "lqc2		vf2, 0x00(%2)	\n"
    "vopmula.xyz		ACC, vf1, vf2	\n"
    "vopmsub.xyz		vf2, vf2, vf1	\n"
    "vsub.w		vf2, vf0, vf0	\n"
    "sqc2		vf2, 0x00(%0)	\n"
#endif
    : : "r" (output), "r" (input0), "r" (input1)
    : "memory"
    );
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


//--------------------------------------------------------
void vector_subtract(VECTOR output, VECTOR input0, VECTOR input1)
{
    asm __volatile__ (
#if __GNUC__ > 3
    "lqc2       $vf1, 0x00(%1)      \n"
    "lqc2       $vf2, 0x00(%2)      \n"
    "vsub.xyz   $vf1, $vf1, $vf2    \n"
    "sqc2       $vf1, 0x00(%0)      \n"
#else
    "lqc2		vf1, 0x00(%1)	    \n"
    "lqc2		vf2, 0x00(%2)	    \n"
    "vsub.xyz   vf1, vf1, vf2       \n"
    "sqc2		vf1, 0x00(%0)	    \n"
#endif
    : : "r" (output), "r" (input0), "r" (input1)
  );
}

//--------------------------------------------------------
void vector_add(VECTOR output, VECTOR input0, VECTOR input1)
{
    asm __volatile__ (
#if __GNUC__ > 3
    "lqc2       $vf1, 0x00(%1)      \n"
    "lqc2       $vf2, 0x00(%2)      \n"
    "vadd.xyz   $vf1, $vf2, $vf1    \n"
    "sqc2       $vf1, 0x00(%0)      \n"
#else
    "lqc2		vf1, 0x00(%1)	    \n"
    "lqc2		vf2, 0x00(%2)	    \n"
    "vadd.xyz   vf1, vf2, vf1       \n"
    "sqc2		vf1, 0x00(%0)	    \n"
#endif
    : : "r" (output), "r" (input0), "r" (input1)
  );
}

//--------------------------------------------------------
// void vector_negate(VECTOR output, VECTOR input0);

//--------------------------------------------------------
void vector_scale(VECTOR output, VECTOR input0, float scalar)
{
    VECTOR timeVector;
    timeVector[0] = scalar;

    asm __volatile__ (
#if __GNUC__ > 3
    "lqc2       $vf1, 0x00(%1)      \n"
    "lqc2       $vf2, 0x00(%2)      \n"
    "vmulx.xyz  $vf1, $vf1, $vf2x   \n"
    "sqc2       $vf1, 0x00(%0)      \n"
#else
    "lqc2		vf1, 0x00(%1)	    \n"
    "lqc2		vf2, 0x00(%2)	    \n"
    "vmulx.xyz  vf1, vf1, vf2x      \n"
    "sqc2		vf1, 0x00(%0)	    \n"
#endif
    : : "r" (output), "r" (input0), "r" (timeVector)
  );
}

//--------------------------------------------------------
void vector_print(VECTOR input0)
{
	printf("(%s%d.%05d, %s%d.%05d, %s%d.%05d, %s%d.%05d)",
		(input0[0] < 0 ? "-" : "+"), (int)fabsf(input0[0]), ((int)(fabsf(input0[0]) * 100000)) % 100000,
		(input0[1] < 0 ? "-" : "+"), (int)fabsf(input0[0]), ((int)(fabsf(input0[1]) * 100000)) % 100000,
		(input0[2] < 0 ? "-" : "+"), (int)fabsf(input0[0]), ((int)(fabsf(input0[2]) * 100000)) % 100000,
		(input0[3] < 0 ? "-" : "+"), (int)fabsf(input0[0]), ((int)(fabsf(input0[3]) * 100000)) % 100000);
}

//--------------------------------------------------------
void quaternion_fromeuler(VECTOR output, VECTOR input0)
{
    float cy = cosf(input0[0] * 0.5);
    float sy = sinf(input0[0] * 0.5);
    float cp = cosf(input0[1] * 0.5);
    float sp = sinf(input0[1] * 0.5);
    float cr = cosf(input0[2] * 0.5);
    float sr = sinf(input0[2] * 0.5);

    output[3] = cr * cp * cy + sr * sp * sy;
    output[0] = sr * cp * cy - cr * sp * sy;
    output[1] = cr * sp * cy + sr * cp * sy;
    output[2] = cr * cp * sy - sr * sp * cy;
}

//--------------------------------------------------------
void quaternion_toeuler(VECTOR output, VECTOR input0)
{
    VECTOR q;
    float sqw = input0[3] * input0[3];
    float sqx = input0[0] * input0[0];
    float sqy = input0[1] * input0[1];
    float sqz = input0[2] * input0[2];
    float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
    float test = input0[0] * input0[3] - input0[1] * input0[2];

    if (test > 0.4995 * unit)
    { // singularity at north pole
        output[1] = clampAngle(2 * atan2f(input0[1], input0[0]));
        output[0] = MATH_PI / 2;
        output[2] = 0;
        return;
    }
    if (test < -0.4995 * unit)
    { // singularity at south pole
        output[1] = clampAngle(-2 * atan2f(input0[1], input0[0]));
        output[0] = -MATH_PI / 2;
        output[2] = 0;
        return;
    }
    
    q[0] = input0[3];
    q[1] = input0[2];
    q[2] = input0[0];
    q[3] = input0[1];
    output[1] = clampAngle(atan2f(2 * q[0] * q[3] + 2 * q[1] * q[2], 1 - 2 * (q[2] * q[2] + q[3] * q[3])));
    output[0] = clampAngle(asinf(2 * (q[0] * q[2] - q[3] * q[1])));
    output[2] = clampAngle(atan2f(2 * q[0] * q[1] + 2 * q[2] * q[3], 1 - 2 * (q[1] * q[1] + q[2] * q[2])));
    output[3] = 0;
}

//--------------------------------------------------------
void quaternion_lookrotation(VECTOR output, VECTOR input0, VECTOR input1)
{
    MATRIX m;
    VECTOR forward, left, up;

    vector_normalize(forward, input0);
    vector_normalize(up, input1);
    vector_outerproduct(left, up, forward);
    vector_normalize(left, left);
    
    // adapted source
    float trace = left[0] + up[1] + forward[2];
    if (trace > 0.0)
    {
        float s = 0.5 / sqrtf(trace + 1.0);
        output[3] = 0.25 / s;
        output[0] = (up[2] - forward[1]) * s;
        output[1] = (forward[0] - left[2]) * s;
        output[2] = (left[1] - up[0]) * s;
    }
    else
    {
        if (left[0] > up[1] && left[0] > forward[2])
        {
            float s = 2.0 * sqrtf(1.0 + left[0] - up[1] - forward[2]);
            output[3] = (up[2] - forward[1]) / s;
            output[0] = 0.25 * s;
            output[1] = (up[0] + left[1]) / s;
            output[2] = (forward[0] + left[2]) / s;
        }
        else if (up[1] > forward[2])
        {
            float s = 2.0 * sqrtf(1.0 + up[1] - left[0] - forward[2]);
            output[3] = (forward[0] - left[2]) / s;
            output[0] = (up[0] + left[1]) / s;
            output[1] = 0.25 * s;
            output[2] = (forward[1] + up[2]) / s;
        }
        else
        {
            float s = 2.0 * sqrtf(1.0 + forward[2] - left[0] - up[1]);
            output[3] = (left[1] - up[0]) / s;
            output[0] = (forward[0] + left[2]) / s;
            output[1] = (forward[1] + up[2]) / s;
            output[2] = 0.25 * s;
        }
    }
}

//--------------------------------------------------------
void quaternion_multiply(VECTOR output, VECTOR input1, VECTOR input0)
{
    VECTOR temp;
    temp[3] = input0[3] * input1[3] - input0[0] * input1[0] - input0[1] * input1[1] - input0[2] * input1[2];
    temp[0] = input0[3] * input1[0] + input0[0] * input1[3] - input0[1] * input1[2] + input0[2] * input1[1];
    temp[1] = input0[3] * input1[1] + input0[0] * input1[2] + input0[1] * input1[3] - input0[2] * input1[0];
    temp[2] = input0[3] * input1[2] - input0[0] * input1[1] + input0[1] * input1[0] + input0[2] * input1[3];
    vector_copy(output, temp);
}

//--------------------------------------------------------
void quaternion_frommatrix(VECTOR output, MATRIX input0)
{
    float s = 0;
    float t = input0[0] + input0[5] + input0[10];
    // we protect the division by s by ensuring that s>=1
    if (t > 0) { // by w
        float s = sqrtf(t + 1);
        output[3] = 0.5 * s;
        s = 0.5 / s;                 
        output[0] = (input0[(4 * 2) + 1] - input0[(4 * 1) + 2]) * s;
        output[1] = (input0[(4 * 0) + 2] - input0[(4 * 2) + 0]) * s;
        output[2] = (input0[(4 * 1) + 0] - input0[(4 * 0) + 1]) * s;
    } else if ((input0[(4 * 0) + 0] > input0[(4 * 1) + 1]) && (input0[(4 * 0) + 0] > input0[(4 * 2) + 2])) { // by x
        float s = sqrtf(1 + input0[(4 * 0) + 0] - input0[(4 * 1) + 1] - input0[(4 * 2) + 2]); 
        output[0] = s * 0.5; 
        s = 0.5 / s;
        output[1] = (input0[(4 * 1) + 0] + input0[(4 * 0) + 1]) * s;
        output[2] = (input0[(4 * 0) + 2] + input0[(4 * 2) + 0]) * s;
        output[3] = (input0[(4 * 2) + 1] - input0[(4 * 1) + 2]) * s;
    } else if (input0[(4 * 1) + 1] > input0[(4 * 2) + 2]) { // by y
        float s = sqrtf(1 + input0[(4 * 1) + 1] - input0[(4 * 0) + 0] - input0[(4 * 2) + 2]); 
        output[1] = s * 0.5; 
        s = 0.5 / s;
        output[0] = (input0[(4 * 1) + 0] + input0[(4 * 0) + 1]) * s;
        output[2] = (input0[(4 * 2) + 1] + input0[(4 * 1) + 2]) * s;
        output[3] = (input0[(4 * 0) + 2] - input0[(4 * 2) + 0]) * s;
    } else { // by z
        float s = sqrtf(1 + input0[(4 * 2) + 2] - input0[(4 * 0) + 0] - input0[(4 * 1) + 1]); 
        output[2] = s * 0.5; 
        s = 0.5 / s;
        output[0] = (input0[(4 * 0) + 2] + input0[(4 * 2) + 0]) * s;
        output[1] = (input0[(4 * 2) + 1] + input0[(4 * 1) + 2]) * s;
        output[3] = (input0[(4 * 1) + 0] - input0[(4 * 0) + 1]) * s;
    }
}

//--------------------------------------------------------
void matrix_fromquaternion(MATRIX output, VECTOR input0)
{
    float sqx = input0[0] * input0[0];
    float sqy = input0[1] * input0[1];
    float sqz = input0[2] * input0[2];
    float sqw = input0[3] * input0[3];

    output[(0 * 4) + 0] = 2 * (sqx + sqy) - 1;
    output[(0 * 4) + 1] = 2 * (input0[1]*input0[2] - input0[0]*input0[3]);
    output[(0 * 4) + 2] = 2 * (input0[1]*input0[3] + input0[0]*input0[2]);
    output[(0 * 4) + 3] = 0;

    output[(1 * 4) + 0] = 2 * (input0[1]*input0[2] + input0[0]*input0[3]);
    output[(1 * 4) + 1] = 2 * (sqx + sqz) - 1;
    output[(1 * 4) + 2] = 2 * (input0[2]*input0[3] - input0[0]*input0[1]);
    output[(1 * 4) + 3] = 0;
    
    output[(2 * 4) + 0] = 2 * (input0[1]*input0[3] - input0[0]*input0[2]);
    output[(2 * 4) + 1] = 2 * (input0[2]*input0[3] + input0[0]*input0[1]);
    output[(2 * 4) + 2] = 2 * (sqx + sqw) - 1;
    output[(2 * 4) + 3] = 0;
    
    output[(3 * 4) + 0] = 0;
    output[(3 * 4) + 1] = 0;
    output[(3 * 4) + 2] = 0;
    output[(3 * 4) + 3] = 1;
    
}

//--------------------------------------------------------
void matrix_toeuler(VECTOR output, MATRIX input0)
{
    float m11 = input0[0],m12 = input0[1],m13 = input0[2];
    float m21 = input0[4],m22 = input0[5],m23 = input0[6];
    float m31 = input0[8],m32 = input0[9],m33 = input0[10];

    output[0] = atan2f(m23, m33);
    output[1] = -atan2f(-m13, sqrtf(powf(m11,2) + powf(m12,2)));
    float s = sinf(output[0]);
    float c = cosf(output[0]);
    output[2] = atan2f(s*m31 - c*m21, c*m22 - s*m32);
    output[3] = 0;
}

//--------------------------------------------------------
void matrix_fromrows(MATRIX output, VECTOR input0, VECTOR input1, VECTOR input2, VECTOR input3)
{
    vector_copy((float*)(&output[0]), input0);
    vector_copy((float*)(&output[4]), input1);
    vector_copy((float*)(&output[8]), input2);
    vector_copy((float*)(&output[12]), input3);
}

//--------------------------------------------------------
void matrix_multiply(MATRIX output, MATRIX input0, MATRIX input1)
{
    asm __volatile__ (
#if __GNUC__ > 3
    "lqc2		$vf1, 0x00(%1)	\n"
    "lqc2		$vf2, 0x10(%1)	\n"
    "lqc2		$vf3, 0x20(%1)	\n"
    "lqc2		$vf4, 0x30(%1)	\n"
    "lqc2		$vf5, 0x00(%2)	\n"
    "lqc2		$vf6, 0x10(%2)	\n"
    "lqc2		$vf7, 0x20(%2)	\n"
    "lqc2		$vf8, 0x30(%2)	\n"
    "vmulax.xyzw		$ACC, $vf5, $vf1\n"
    "vmadday.xyzw	$ACC, $vf6, $vf1\n"
    "vmaddaz.xyzw	$ACC, $vf7, $vf1\n"
    "vmaddw.xyzw		$vf1, $vf8, $vf1\n"
    "vmulax.xyzw		$ACC, $vf5, $vf2\n"
    "vmadday.xyzw	$ACC, $vf6, $vf2\n"
    "vmaddaz.xyzw	$ACC, $vf7, $vf2\n"
    "vmaddw.xyzw		$vf2, $vf8, $vf2\n"
    "vmulax.xyzw		$ACC, $vf5, $vf3\n"
    "vmadday.xyzw	$ACC, $vf6, $vf3\n"
    "vmaddaz.xyzw	$ACC, $vf7, $vf3\n"
    "vmaddw.xyzw		$vf3, $vf8, $vf3\n"
    "vmulax.xyzw		$ACC, $vf5, $vf4\n"
    "vmadday.xyzw	$ACC, $vf6, $vf4\n"
    "vmaddaz.xyzw	$ACC, $vf7, $vf4\n"
    "vmaddw.xyzw		$vf4, $vf8, $vf4\n"
    "sqc2		$vf1, 0x00(%0)	\n"
    "sqc2		$vf2, 0x10(%0)	\n"
    "sqc2		$vf3, 0x20(%0)	\n"
    "sqc2		$vf4, 0x30(%0)	\n"
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
    : "memory"
    );
}

//--------------------------------------------------------
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

//--------------------------------------------------------
void matrix_unit(MATRIX output)
{
    // Create a unit matrix.
    memset(output, 0, sizeof(MATRIX));
    output[0x00] = 1.00f;
    output[0x05] = 1.00f;
    output[0x0A] = 1.00f;
    output[0x0F] = 1.00f;
}

//--------------------------------------------------------
float matrix_determinant(MATRIX input0)
{
    return
        input0[0] * (input0[5]*input0[10]*input0[15] + input0[6]*input0[11]*input0[13] + input0[7]*input0[9]*input0[14]
                    -input0[7]*input0[10]*input0[13] - input0[6]*input0[9]*input0[15] - input0[5]*input0[11]*input0[14])

    -   input0[4] * (input0[1]*input0[10]*input0[15] + input0[2]*input0[11]*input0[13] + input0[3]*input0[9]*input0[14]
                    -input0[3]*input0[10]*input0[13] - input0[2]*input0[9]*input0[15] - input0[1]*input0[11]*input0[14])

    +   input0[8] * (input0[1]*input0[6]*input0[15] + input0[2]*input0[7]*input0[13] + input0[3]*input0[5]*input0[14]
                    -input0[3]*input0[6]*input0[13] - input0[2]*input0[5]*input0[15] - input0[1]*input0[7]*input0[14])

    -   input0[12] *(input0[1]*input0[6]*input0[11] + input0[2]*input0[7]*input0[9] + input0[3]*input0[5]*input0[10]
                    -input0[3]*input0[6]*input0[9] - input0[2]*input0[5]*input0[11] - input0[1]*input0[7]*input0[10]);
}
