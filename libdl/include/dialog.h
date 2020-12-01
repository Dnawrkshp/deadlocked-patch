/***************************************************
 * FILENAME :		sound.h
 * 
 * DESCRIPTION :
 * 		Contains sound specific offsets and structures for Deadlocked.
 * 
 * NOTES :
 * 
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#ifndef _LIBDL_DIALOG_H_
#define _LIBDL_DIALOG_H_

#include <tamtypes.h>

/*
 *
 */
enum DialogIds
{
    DIALOG_ID_PLEASE_TAKE_YOUR_TIME = 0x2C92,
    DIALOG_ID_ARE_YOU_DONE_YET = 0x2C93,
    // If you're finished reviewing your rankings, please leave. I have a date...
    DIALOG_ID_FINISHED_REVIEWING_RANKINGS = 0x2C94,
    // Please review your rankings at your leisure
    DIALOG_ID_REVIEW_YOUR_RANKINGS = 0x2C95,
    DIALOG_ID_HELLO_HERO = 0x2C96,
};

void playDialogSound(int dialogId, int arg2);


#endif // _LIBDL_DIALOG_H_
