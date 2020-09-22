#include "math.h"

//--------------------------------------------------------
#define MATH_COSF_FUNC                  (0x00135878)
#define MATH_SQRTF_FUNC                 (0x00136C90) 
#define MATH_POWF_FUNC                  (0x00136788)
#define MATH_FABS_FUNC                  (0x00135300)
#define MATH_FABSF_FUNC                 (0x00135960)

//--------------------------------------------------------
float cosf(float theta)
{
    return ((float (*)(float))MATH_COSF_FUNC)(theta);
}

//--------------------------------------------------------
float sinf(float theta)
{
    return ((float (*)(float))MATH_COSF_FUNC)(theta - MATH_PI);
}

//--------------------------------------------------------
float sqrtf(float f)
{
    return ((float (*)(float))MATH_SQRTF_FUNC)(f);
}

//--------------------------------------------------------
float powf(float base, float exp)
{
    return ((float (*)(float, float))MATH_POWF_FUNC)(base, exp);
}

//--------------------------------------------------------
float fabsf(float f)
{
    return ((float (*)(float))MATH_FABSF_FUNC)(f);
}

//--------------------------------------------------------
float lerpf(float a, float b, float t)
{
    return (b-a)*t + a;
}

//--------------------------------------------------------
float lerpfAngle(float a, float b, float t)
{
    if (fabsf(a-b) < MATH_PI)
        return clampAngle(lerpf(a, b, t));
    else if (a < b)
        return clampAngle(lerpf(a, b - MATH_TAU, t));
    else
        return clampAngle(lerpf(a, b + MATH_TAU, t));
}

//--------------------------------------------------------
float clampAngle(float theta)
{
    while (theta > MATH_PI)
        theta -= MATH_TAU;
    while (theta < -MATH_PI)
        theta += MATH_TAU;

    return theta;
}
