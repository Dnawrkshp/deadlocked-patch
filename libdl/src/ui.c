#include "ui.h"

#define UI_ACTIVE_ID                            (*(int*)0x003434B8)
#define UI_DIALOG_FUNC                          (0x00719A28)
#define UI_DIALOG_A0                            ((void*)0x011C7000)

int uiGetActive(void)
{
    return UI_ACTIVE_ID;
}

int uiShowYesNoDialog(const char * title, const char * description)
{
    return ((int (*)(void *, const char *, const char *, int, int, int, int))UI_DIALOG_FUNC)(UI_DIALOG_A0, title, description, 3, 0, 1, 0);
}

int uiShowOkDialog(const char * title, const char * description)
{
    return ((int (*)(void *, const char *, const char *, int, int, int, int))UI_DIALOG_FUNC)(UI_DIALOG_A0, title, description, 4, 0, 0, 0);
}
