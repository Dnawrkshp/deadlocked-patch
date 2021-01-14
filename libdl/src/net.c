#include <tamtypes.h>
#include "stdio.h"
#include "net.h"
#include "string.h"


#define GET_MEDIUS_APP_HANDLER_HOOK         (*(u32*)0x01EAAB10)
#define NET_LOBBY_CONNECTION                ((void*)(*(u32*)0x001AF91C))

NET_CALLBACK_DELEGATE callbacks[256] = {};

int customMsgHandler(void * connection, u64 a1, u64 a2, u8 * data)
{
    u8 id = data[0];

    NET_CALLBACK_DELEGATE callback = callbacks[id];
    if (callback)
        return 4 + callback(connection, (void*)(data + 4));
    else
        printf("unhandled custom message id:%d\n", id);

    return 4;
}

int mediusMsgHandler(u64 a0, u64 a1, u32 * callback, u64 a3, u64 t0)
{
    if (a3 == NET_CUSTOM_MESSAGE_CLASS && t0 == NET_CUSTOM_MESSAGE_ID)
    {
        *callback = (u32)&customMsgHandler;
        return 0;
    }

    return a0 == 0;
}

void installCustomMsgHook(void)
{
    GET_MEDIUS_APP_HANDLER_HOOK = 0x08000000 | ((u32)&mediusMsgHandler / 4);
}

void InstallCustomMsgHandler(u8 id, NET_CALLBACK_DELEGATE callback)
{
    // install hook
    installCustomMsgHook();

    // install callback
    callbacks[id] = callback;
}

int SendMediusAppMessage(void * connection, int msgClass, int msgId, int msgSize, void * payload)
{
    return ((int (*)(int transport, void * connection, u64 a2, int msgClass, int msgId, int msgSize, void * payload))0x01E9E198)(0x40, connection, 0xFFFF, msgClass, msgId, msgSize, payload);
}

int SendCustomAppMessage(void * connection, u8 customMsgId, int msgSize, void * payload)
{
    u8 buffer[512];

    if (payload && msgSize > 0)
        memcpy(buffer + 4, payload, msgSize);
        
    buffer[0] = customMsgId;
    SendMediusAppMessage(connection, NET_CUSTOM_MESSAGE_CLASS, NET_CUSTOM_MESSAGE_ID, msgSize + 4, buffer);
}

void* netGetLobbyServerConnection(void)
{
    return NET_LOBBY_CONNECTION;
}
