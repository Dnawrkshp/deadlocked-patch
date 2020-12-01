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
    // Alright let's get you familiar with the basic features of that suit
    DIALOG_ID_2900 = 0x2900,
    // Use directional buttons to activate bot command menu
    DIALOG_ID_2901 = 0x2901,
    // To use your swingshot
    DIALOG_ID_2903 = 0x2903,

    // Let's do some damage
    DIALOG_ID_29DD = 0x29DD,

    // Watch your six
    DIALOG_ID_2A31 = 0x2A31,

    // ACE:         0x2AC6 - 0x2AE2
    // REACTOR:     0x2AE3 - 0x2AF0
    // SHELLSHOCK:  0x2AF1 - 0x2AFE
    // VOX:         0x2AFF - 0x2B43
    // AL:          0x
    // CLANK:       0x2B6F - 0x2B81
    // DALLAS+JUAN: 0x2B82 - 0x2C35

    DIALOG_ID_JUANITA_LAUGH = 0x2BA1,
    DIALOG_ID_DALLAS_PICKS_WEAPONS = 0x2BB6,

    DIALOG_ID_LETS_SEE_HOW_HE_DOES_WITH_THIS = 0x2BBC,
    DIALOG_ID_TRY_THIS_ONE_ON_FOR_SIZE = 0x2BBD,
    DIALOG_ID_TEAM_DEADSTAR = 0x2C0E,

    
    // ANNOUNCEMENTS:    0x2C36 - 0x2C48
    DIALOG_ID_BULK_LOGAN = 0x2C39,
    DIALOG_ID_KEEP_THE_ACTION_LIVELY = 0x2C44,

    // MP:                  0x2C49 - 2C72

    // STAT MACHINE:        0x2C73 - 0x2CAE
    DIALOG_ID_REFLECT_TERRIBLE_YOU_ARE = 0x2C7B,
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
