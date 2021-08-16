#include <libdl/pad.h>
#include <libdl/ui.h>
#include <libdl/graphics.h>

int isSettingsActive = 0;
u32 padPointer = 0;

u32 colorBg = 0x80404040;
RECT rectBgBox = {
  { 0.2, 0.25 },
  { 0.8, 0.25 },
  { 0.2, 0.85 },
  { 0.8, 0.85 }
};

enum ModType
{
  MODTYPE_TOGGLE = 0,
};

typedef struct
{
  enum ModType type;
  char name[16];
  int * value;
  int minValue;
  int maxValue;
} MenuElem_t;

typedef struct
{
  int disableFramelimiter;
  int enableGamemodeAnnouncements;
  int enableSpectate;
  int enableSingleplayerMusic;
} PatchSettings_t;

//------------------------------------------------------------------------------
void drawBackgroundBox(void)
{
	gfxScreenSpaceBox(&rectBgBox, colorBg, colorBg, colorBg, colorBg);
}

//------------------------------------------------------------------------------
void settingsMenuDisable(void)
{
  isSettingsActive = 0;

  // re-enable pad
  *(u32*)0x0021DDCC = padPointer;
}

//------------------------------------------------------------------------------
void settingsMenuEnable(void)
{
  // enable
  isSettingsActive = 1;

  // prevent pad from affecting menus
  padPointer = *(u32*)0x0021DDCC;
  *(u32*)0x0021DDCC = 0;
}

//------------------------------------------------------------------------------
void onSettingsOnlineMenu(void)
{
  if (isSettingsActive)
  {
    // draw background
    drawBackgroundBox();


    if (padGetButtonDown(0, PAD_L3 | PAD_R3) > 0)
    {
      settingsMenuDisable();
    }
  }
  else if (padGetButtonDown(0, PAD_L3 | PAD_R3) > 0)
  {
    settingsMenuEnable();
  }
}

