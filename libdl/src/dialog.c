#include "game.h"
#include "dialog.h"

/*
 * NAME :		playDialogSound
 * 
 * DESCRIPTION :
 * 			Plays the given dialog sound.
 * 
 * NOTES :
 * 
 * ARGS : 
 *          dialogId:           Dialog sound id.
 *          arg2    :           No idea.
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void playDialogSound(int dialogId, int arg2)
{
    if (isInGame())
    {
        ((void (*)(u64, u64, u64, u64, float, float, float, float))0x004E3688)(dialogId, arg2, 2, 0, 0, 1, 0, 64);
    }
    else
    {
        ((void (*)(u64, u64))0x5C97D8)(dialogId, arg2);
    }
}
