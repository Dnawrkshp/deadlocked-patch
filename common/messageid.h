
/***************************************************
 * FILENAME :		messageid.h
 * 
 * DESCRIPTION :
 * 		
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _MESSAGEID_H_
#define _MESSAGEID_H_

/*
 * NAME :		CustomMessageId
 * 
 * DESCRIPTION :
 * 			Contains the different message ids.
 * 
 * NOTES :
 * 
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
enum CustomMessageId
{
    UNUSED = 0,

    /*
     * Info on custom map.
     */
    CUSTOM_MSG_ID_SET_MAP_OVERRIDE = 1,

    /*
     * Sent to the server when the client wants the map loader irx modules.
     */
    CUSTOM_MSG_ID_CLIENT_REQUEST_MAP_IRX_MODULES = 2,

    /*
     * Sent after the server has sent all the map loader irx modules.
     */
    CUSTOM_MSG_ID_SERVER_SENT_MAP_IRX_MODULES = 3,

    /*
     * Sent in response to CUSTOM_MSG_ID_SET_MAP_OVERRIDE by the client.
     */
    CUSTOM_MSG_ID_SET_MAP_OVERRIDE_RESPONSE = 4,

    /*
     * Sent by the host when a game lobby has started.
     */
    CUSTOM_MSG_ID_GAME_LOBBY_STARTED = 5,

    /*
     *
     */
    CUSTOM_MSG_ID_SEARCH_AND_DESTROY_SET_OUTCOME = 6,

};

#endif // _MESSAGEID_H_
