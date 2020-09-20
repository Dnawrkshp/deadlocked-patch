#include "stdio.h"

#define SPRINTF_FUNC                    (0x0011D920)
#define PRINTF_FUNC                     (0x0011D5D8)

//--------------------------------------------------------
int (*sprintf)(char * buffer, const char * format, ...) = (int (*)(char * buffer, const char * format, ...))SPRINTF_FUNC;

//--------------------------------------------------------
int (*printf)(const char * format, ...) = (int (*)(const char * format, ...))PRINTF_FUNC;
