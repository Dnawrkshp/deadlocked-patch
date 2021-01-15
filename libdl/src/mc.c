#include "mc.h"
#include <kernel.h>
#include "string.h"

#define MC_OPEN_FUNC                    (0x0013A160)
#define MC_CLOSE_FUNC                   (0x0013A2C0)
#define MC_SEEK_FUNC                    (0x0013A380)
#define MC_READ_FUNC                    (0x0013A510)
#define MC_WRITE_FUNC                   (0x0013A628)
#define MC_SEMA                         (*(u32*)0x00167BFC)
#define MC_RESULT                       (*(s32*)0x001AEE40)

//--------------------------------------------------------
int McOpen(int port, int slot, const char *name, int mode)
{
    return ((int (*)(int,int,const char*,int))MC_OPEN_FUNC)(port, slot, name, mode);
}

//--------------------------------------------------------
int McClose(int fd)
{
    return ((int (*)(int))MC_CLOSE_FUNC)(fd);
}

//--------------------------------------------------------
int McSeek(int fd, int offset, int origin)
{
    return ((int (*)(int, int, int))MC_SEEK_FUNC)(fd, offset, origin);
}

//--------------------------------------------------------
int McRead(int fd, void *buffer, int size)
{
    return ((int (*)(int, void *, int))MC_READ_FUNC)(fd, buffer, size);
}

//--------------------------------------------------------
int McWrite(int fd, const void *buffer, int size)
{
    return ((int (*)(int, const void *, int))MC_WRITE_FUNC)(fd, buffer, size);
}

//--------------------------------------------------------
int McPollSema(void)
{
    if (*(u32*)0x00171B04 != 0)
    {
        if (((int (*)(int,int,int))0x0013A7A0)(1, 0x171AF0, 0x171AF4) != 1)
            return -1;

        
    }
    
    return *(int*)0x171AF4;
}

//--------------------------------------------------------
int McGetResult(void)
{
    return MC_RESULT;
}
