#include "stdio.h"
#include "string.h"

#define SPRINTF_FUNC                    (0x0011D920)
#define PRINTF_FUNC                     (0x0011D5D8)
#define MEMSET_FUNC                     (0x0011A518)
#define MEMCPY_FUNC                     (0x0011A370)
#define STRNCPY_FUNC                    (0x0011AFC8)

//--------------------------------------------------------
int (*sprintf)(char * buffer, const char * format, ...) = (int (*)(char * buffer, const char * format, ...))SPRINTF_FUNC;

//--------------------------------------------------------
int (*printf)(const char * format, ...) = (int (*)(const char * format, ...))PRINTF_FUNC;

//--------------------------------------------------------
inline void * memset(void * ptr, int value, int num)
{
    return ((void*(*)(void*,int,int))MEMSET_FUNC)(ptr, value, num);
}

//--------------------------------------------------------
inline void * memcpy(void * destination, const void * source, int num)
{
    return ((void*(*)(void*,const void*,int))MEMCPY_FUNC)(destination, source, num);
}

//--------------------------------------------------------
inline char * strncpy(char *dest, const char *src, int n)
{
    return ((void*(*)(char*,const char*,int))STRNCPY_FUNC)(dest, src, n);
}
