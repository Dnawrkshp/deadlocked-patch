#include "patch.h"
#include <sifrpc.h>
#include <sifcmd.h>
#include <iopheap.h>
#include <iopcontrol.h>

// 
extern int _SifLoadModuleBuffer(void *ptr, int arg_len, const char *args, int *modres);

//------------------------------------------------------------------------------
void patch(void * funcPtr, u32 addr)
{
	// 
	*(u32*)funcPtr = 0x08000000 | (addr / 4);
	*((u32*)funcPtr + 1) = 0;
}

/*
 * NAME :		patchSifRpc
 * 
 * DESCRIPTION :
 * 			Patches sif rpc functions to use game's.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void patchSifRpc(void)
{
    // patch sif rpc
	patch(&SifInitRpc, 0x00129D90);
	patch(&SifExitRpc, 0x00129F30);
	patch(&SifRpcGetOtherData, 0x0012A270);
	patch(&SifBindRpc, 0x0012A538);
	patch(&SifCallRpc, 0x0012A718);
	patch(&SifCheckStatRpc, 0x0012A918);
	patch(&SifSetRpcQueue, 0x0012A958);
	patch(&SifInitIopHeap, 0x0012C1B8);
	patch(&SifAllocIopHeap, 0x0012C240);
	patch(&SifFreeIopHeap, 0x0012C3A8);
	patch(&SifIopReset, 0x0012CC30);
	patch(&SifIopSync, 0x0012CDB0);
	patch(&_SifLoadModuleBuffer, 0x0012CBC0);
}
