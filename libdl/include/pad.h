/***************************************************
 * FILENAME :		pad.h
 * 
 * DESCRIPTION :
 * 		Contains pad specific offsets and structures for Deadlocked.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _PAD_H
#define _PAD_H

#include <tamtypes.h>

#define PAD_PORT_MAX        2

#define PAD_LEFT            0x0080
#define PAD_DOWN            0x0040
#define PAD_RIGHT           0x0020
#define PAD_UP              0x0010
#define PAD_START           0x0008
#define PAD_R3              0x0004
#define PAD_L3              0x0002
#define PAD_SELECT          0x0001
#define PAD_SQUARE          0x8000
#define PAD_CROSS           0x4000
#define PAD_CIRCLE          0x2000
#define PAD_TRIANGLE        0x1000
#define PAD_R1              0x0800
#define PAD_L1              0x0400
#define PAD_R2              0x0200
#define PAD_L2              0x0100

typedef struct padButtonStatus
{
    unsigned char ok;
    unsigned char mode;
    unsigned short btns;
    // joysticks
    unsigned char rjoy_h;
    unsigned char rjoy_v;
    unsigned char ljoy_h;
    unsigned char ljoy_v;
    // pressure mode
    unsigned char right_p;
    unsigned char left_p;
    unsigned char up_p;
    unsigned char down_p;
    unsigned char triangle_p;
    unsigned char circle_p;
    unsigned char cross_p;
    unsigned char square_p;
    unsigned char l1_p;
    unsigned char r1_p;
    unsigned char l2_p;
    unsigned char r2_p;
} __attribute__((packed)) PadButtonStatus;

typedef struct PadHistory
{
    u16 btns;
    u8 rjoy_h;
    u8 rjoy_v;
    u8 ljoy_h;
    u8 ljoy_v;
    short id;
} PadHistory;

/*
 * NAME :		padGetButton
 * 
 * DESCRIPTION :
 * 			Returns 1 when the user is pressing the given button combination.
 *          Returns negative on failure.
 * 
 * NOTES :
 * 
 * ARGS : 
 *          port:                       Which controller port to read.
 *          buttonMask:                 Buttons to check.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int padGetButton(int port, u16 buttonMask);

/*
 * NAME :		padGetButtonDown
 * 
 * DESCRIPTION :
 * 			Returns 1 during the frame that the user starts pressing the given button combination.
 *          Returns negative on failure.
 * 
 * NOTES :
 * 
 * ARGS : 
 *          port:                       Which controller port to read.
 *          buttonMask:                 Buttons to check.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int padGetButtonDown(int port, u16 buttonMask);

/*
 * NAME :		padGetButtonUp
 * 
 * DESCRIPTION :
 * 			Returns 1 during the frame that the user releases the given button combination.
 *          Returns negative on failure.
 * 
 * NOTES :
 * 
 * ARGS : 
 *          port:                       Which controller port to read.
 *          buttonMask:                 Buttons to check.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int padGetButtonUp(int port, u16 buttonMask);

/*
 * NAME :		padResetInput
 * 
 * DESCRIPTION :
 * 			Resets the given ports inputs.
 * 
 * NOTES :
 * 
 * ARGS : 
 *          port:                       Which controller port to reset.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void padResetInput(int port);

#endif // _PAD_H
