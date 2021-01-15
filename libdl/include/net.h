/***************************************************
 * FILENAME :		net.h
 * 
 * DESCRIPTION :
 * 		
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _LIBDL_NET_H_
#define _LIBDL_NET_H_

#include <tamtypes.h>


#define NET_CUSTOM_MESSAGE_CLASS                    (0)
#define NET_CUSTOM_MESSAGE_ID                       (7)

typedef int (*NET_CALLBACK_DELEGATE)(void * connection, void * data);


void InstallCustomMsgHandler(u8 id, NET_CALLBACK_DELEGATE callback);

int SendMediusAppMessage(void * connection, int msgClass, int msgId, int msgSize, void * payload);
int SendCustomAppMessage(void * connection, u8 customMsgId, int msgSize, void * payload);


void* netGetLobbyServerConnection(void);

#endif // _LIBDL_NET_H_
