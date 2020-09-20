#include "sha1.h"

//--------------------------------------------------------
#define SHA1_FUNC                           (0x01EB30C8)

/*
 * Computes the SHA1 hash of the input and stores outSize number of bytes of the hash in the output.
 */
int sha1(const void * inBuffer, int inSize, void * outBuffer, int outSize)
{
    return ((int(*)(const void *, int, void *, int))SHA1_FUNC)(inBuffer, inSize, outBuffer, outSize);
}
