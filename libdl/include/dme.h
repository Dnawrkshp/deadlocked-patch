/***************************************************
 * FILENAME :		dme.h
 * 
 * DESCRIPTION :
 * 		
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _LIBDL_DME_H_
#define _LIBDL_DME_H_

#include <tamtypes.h>

enum DmePacketTypeIds
{
    DME_APPMSG_CUSTOM =             0x07,
    DME_APPMSG_STATEUPDATE =        0x09,
};

#endif // _LIBDL_DME_H_
