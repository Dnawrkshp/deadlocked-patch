#include "string.h"


//--------------------------------------------------------
#define MEMCPY_FUNC                     (0x0011A370)
#define MEMSET_FUNC                     (0x0011A518)
#define MEMCMP_FUNC                     (0x0011A2DC)

#define SPRINTF_FUNC                    (0x0011D920)
#define PRINTF_FUNC                     (0x0011D5D8)

#define STRLEN_FUNC                     (0x0011AB04)


//--------------------------------------------------------
void * memcpy(void * dest, const void * src, unsigned int size)
{
    return ((void * (*)(void *, const void *, unsigned int))MEMCPY_FUNC)(dest, src, size);
}

//--------------------------------------------------------
void * memset(void * ptr, int c, unsigned int size)
{
    return ((void * (*)(void *, int, unsigned int))MEMSET_FUNC)(ptr, c, size);
}

//--------------------------------------------------------
void * memcmp(const void * p, const void * q, unsigned int size)
{
    return ((void * (*)(const void *, const void *, int))MEMCMP_FUNC)(p, q, size);
}

//--------------------------------------------------------
int (*sprintf)(char * buffer, const char * format, ...) = (int (*)(char * buffer, const char * format, ...))PRINTF_FUNC;

//--------------------------------------------------------
int (*printf)(const char * format, ...) = (int (*)(const char * format, ...))PRINTF_FUNC;

//--------------------------------------------------------
unsigned int strlen(const char * s)
{
    return ((unsigned int(*)(const char *))STRLEN_FUNC)(s);
}
