#include "math.h"

//--------------------------------------------------------
#define MATH_COSF_FUNC                  (0x00135878)
#define MATH_ASINF_FUNC                 (0x00136278)
#define MATH_SQRTF_FUNC                 (0x00136C90) 
#define MATH_POWF_FUNC                  (0x00136788)
#define MATH_FABS_FUNC                  (0x00135300)
#define MATH_FABSF_FUNC                 (0x00135960)
#define MATH_FMOD_FUNC                  (0x00135C18)

// https://stackoverflow.com/questions/4633177/c-how-to-wrap-a-float-to-the-interval-pi-pi
double wrapMax(double x, double max)
{
    /* integer math: `(max + x % max) % max` */
    return fmod(max + fmod(x, max), max);
}

// https://stackoverflow.com/questions/4633177/c-how-to-wrap-a-float-to-the-interval-pi-pi
double wrapMinMax(double x, double min, double max)
{
    return min + wrapMax(x - min, max - min);
}

//--------------------------------------------------------
float cosf(float theta)
{
    return ((float (*)(float))MATH_COSF_FUNC)(theta);
}

//--------------------------------------------------------
float sinf(float theta)
{
    return ((float (*)(float))MATH_COSF_FUNC)(clampAngle(theta - (MATH_PI / 2)));
}

//--------------------------------------------------------
float acosf(float v)
{
    return (MATH_PI / 2) - asinf(v);
}

//--------------------------------------------------------
float asinf(float v)
{
    return ((float (*)(float))MATH_ASINF_FUNC)(v);
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
    return wrapMinMax(theta, -MATH_PI, MATH_PI);
}

//--------------------------------------------------------
double fmod(double x, double y)
{
    return ((double (*)(double, double))MATH_FMOD_FUNC)(x, y);
}
