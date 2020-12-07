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

    DIALOG_ID_DALLAS_THIS_IS_DREADZONE_FOLKS = 0x270F,
    DIALOG_ID_JUANITA_HEROS_FIGHT_TOGETHER_DIE_TOGETHER_HORRIBLY = 0x2717,
    DIALOG_ID_DALLAS_SHOWOFF = 0x271E,
    DIALOG_ID_JUANITA_MORON = 0x271F,
    DIALOG_ID_DALLAS_WOAH_THIS_IS_GETTING_INTERESTING = 0x274E,
    DIALOG_ID_CLANK_YOU_HAVE_A_CHOICE_OF_2_PATHS = 0x2766,
    DIALOG_ID_DALLAS_SHOWTIME = 0x2783,
    DIALOG_ID_DALLAS_RATCHET_LAST_WILL_AND_TESTAMENT = 0x2791,
    DIALOG_ID_JUANITA_I_CANT_BEAR_TO_LOOK_YES_I_CAN = 0x2792,
    DIALOG_ID_DALLAS_WHO_PACKED_YOUR_PARACHUTE = 0x27A6,
    DIALOG_ID_DALLAS_CARNAGE_LAST_RELATIONSHIP = 0x27CB,
    DIALOG_ID_DALLAS_LOOK_AT_THAT_LITTLE_GUY_GO = 0x27EF,
    DIALOG_ID_DALLAS_HE_CROSSED_THE_FINISH_LINE = 0x27F4,
    DIALOG_ID_DALLAS_KICKING_PROVERBIAL_BUTT_IDK_WHAT_THAT_MEANS = 0x27FE,
    DIALOG_ID_DALLAS_WHOOPS = 0x2805,
    DIALOG_ID_DALLAS_DARKSTAR_TIGHT_SPOTS_BEFORE = 0x2807,
    DIALOG_ID_JUANITA_THAT_DOESNT_SURPRISE_ME = 0x280B,

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
    DIALOG_ID_DALLAS_MAYDAY_LOMBAX_GOING_DOWN = 0x2B96,
    DIALOG_ID_DALLAS_DING_DONG_BELLS_RINGING_RATCHET_LAND = 0x2B97,
    DIALOG_ID_DALLAS_BET_AGAINST_RATCHET_IM_RICH = 0x2B98,

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
