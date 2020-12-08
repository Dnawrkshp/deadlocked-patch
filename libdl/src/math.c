#include "math.h"

//--------------------------------------------------------
#define MATH_COSF_FUNC                  (0x00135878)
#define MATH_ASINF_FUNC                 (0x00136278)
#define MATH_SQRTF_FUNC                 (0x00136C90) 
#define MATH_POWF_FUNC                  (0x00136788)
#define MATH_FABS_FUNC                  (0x00135300)
#define MATH_FABSF_FUNC                 (0x00135960)
#define MATH_FMOD_FUNC                  (0x00135C18)

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
float signf(float a)
{
    return (a < 0) ? -1 : 1;
}

//--------------------------------------------------------
float maxf(float a, float b)
{
    return (a < b) ? b : a;
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
float clamp(float v, float min, float max)
{
    if (v < min)
        return min;
    if (v > max)
        return max;

    return v;
}

//--------------------------------------------------------
float clampAngle(float theta)
{
    if (theta > MATH_PI)
        theta -= MATH_TAU;
    else if (theta < -MATH_PI)
        theta += MATH_TAU;

    return theta;
}


//--------------------------------------------------------
// https://gist.github.com/volkansalma/2972237
float atan2f(float y, float x)
{
    //http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
    //Volkan SALMA

    const float ONEQTR_PI = MATH_PI / 4.0;
	const float THRQTR_PI = 3.0 * MATH_PI / 4.0;
	float r, angle;
	float abs_y = fabsf(y) + 1e-10f;      // kludge to prevent 0/0 condition
	if ( x < 0.0f )
	{
		r = (x + abs_y) / (abs_y - x);
		angle = THRQTR_PI;
	}
	else
	{
		r = (x - abs_y) / (x + abs_y);
		angle = ONEQTR_PI;
	}
	angle += (0.1963f * r * r - 0.9817f) * r;
	if ( y < 0.0f )
		return( -angle );     // negate if in quad III or IV
	else
		return( angle );


}
