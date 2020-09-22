#include "dl.h"

// 
extern void padUpdate(void);
extern void playerPadUpdate(void);

/*
 * NAME :		dlPreUpdate
 * 
 * DESCRIPTION :
 * 			Must be called at the beginning of each tick.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void dlPreUpdate(void)
{

}

/*
 * NAME :		dlPostUpdate
 * 
 * DESCRIPTION :
 * 			Must be called at the end of each tick.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
void dlPostUpdate(void)
{
    padUpdate();
    playerPadUpdate();
}
