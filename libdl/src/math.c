#include "math.h"


//--------------------------------------------------------
#define MATH_COSF_FUNC                  (0x00135878)
#define MATH_SQRTF_FUNC                 (0x00136C90) 
#define MATH_POWF_FUNC                  (0x00136788)

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
