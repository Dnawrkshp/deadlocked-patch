#include <libdl/pad.h>
#include <libdl/ui.h>
#include <libdl/graphics.h>
#include <libdl/stdio.h>
#include <libdl/net.h>
#include <libdl/color.h>
#include "include/config.h"
#include "messageid.h"

enum ModType
{
  MODTYPE_NONE = 0,
  MODTYPE_TOGGLE,
  MODTYPE_BUTTON
};

enum ActionType
{
  ACTIONTYPE_DRAW,
  ACTIONTYPE_GETHEIGHT,
  ACTIONTYPE_SELECT,
  ACTIONTYPE_INCREMENT,
  ACTIONTYPE_DECREMENT
};

typedef void (*ActionHandler)(int elementId, int actionType, void * userdata);

typedef struct MenuElem
{
  enum ModType type;
  char name[32];
  int enabled;
  ActionHandler handler;
  void * userdata;
} MenuElem_t;

// config
extern PatchConfig_t config;

// 
int isConfigMenuActive = 0;
int selectedMenuItem = 0;
u32 padPointer = 0;

// constants
const float lineHeight = 0.05;
const char footerText[] = "\x10 SELECT     \x12 BACK";

// menu display properties
const u32 colorBlack = 0x80000000;
const u32 colorBg = 0x80404040;
const u32 colorRed = 0x80000040;
const u32 colorSelected = 0x80606060;
const u32 colorButtonBg = 0x80303030;
const u32 colorButtonFg = 0x80505050;
const u32 colorText = 0x80FFFFFF;
const u32 colorOpenBg = 0x20000000;
RECT rectBgBox = {
  { 0.1, 0.15 },
  { 0.9, 0.15 },
  { 0.1, 0.85 },
  { 0.9, 0.85 }
};
RECT rectOpenBg = {
  { 0.1, 0.75 },
  { 0.5, 0.75 },
  { 0.1, 0.8 },
  { 0.5, 0.8 }
};

//
void configMenuDisable(void);
void configMenuEnable(void);

void mapsActionHandler(int elementId, int actionType, void * userdata);
void toggleActionHandler(int elementId, int actionType, void * userdata);

int mapsHasTriedLoading(void);
int mapsPromptEnableCustomMaps(void);
int mapsDownloadingModules(void);

// menu items
MenuElem_t menuElements[] = {
  { MODTYPE_TOGGLE, "Enable custom maps", 1, mapsActionHandler, NULL },
  { MODTYPE_TOGGLE, "Disable framelimiter", 1, toggleActionHandler, &config.disableFramelimiter },
  { MODTYPE_TOGGLE, "Announcers on all gamemodes", 1, toggleActionHandler, &config.enableGamemodeAnnouncements },
  { MODTYPE_TOGGLE, "Spectate mode", 1, toggleActionHandler, &config.enableSpectate },
  { MODTYPE_TOGGLE, "Singleplayer music", 0, toggleActionHandler, &config.enableSingleplayerMusic },
};

//------------------------------------------------------------------------------
void drawToggleMenuElement(MenuElem_t* element, RECT* rect)
{
  float x,y;
  float lerp = element->enabled ? 0.0 : 0.5;
  u32 color = colorLerp(colorText, 0, lerp);

  // draw name
  x = (rect->TopLeft[0] * SCREEN_WIDTH) + 5;
  y = (rect->TopLeft[1] * 440.0) + 5;
  gfxScreenSpaceText(x, y, 1, 1, color, element->name, -1, 6);

  // draw value
  x = (rect->TopRight[0] * SCREEN_WIDTH) - 5;
  gfxScreenSpaceText(x, y, 1, 1, color, *(char*)element->userdata ? "On" : "Off", -1, 8);
}

//------------------------------------------------------------------------------
void drawButtonMenuElement(MenuElem_t* element, RECT* rect)
{
  float x,y,b = 0.005;
  float lerp = element->enabled ? 0.0 : 0.5;
  u32 color;
  RECT rBg = {
    { rect->TopLeft[0] + 0.05, rect->TopLeft[1] },
    { rect->TopRight[0] - 0.05, rect->TopRight[1] },
    { rect->BottomLeft[0] + 0.05, rect->BottomLeft[1] },
    { rect->BottomRight[0] - 0.05, rect->BottomRight[1] },
  };
  RECT rFg = {
    { rBg.TopLeft[0] + b, rBg.TopLeft[1] + b },
    { rBg.TopRight[0] - b, rBg.TopRight[1] + b },
    { rBg.BottomLeft[0] + b, rBg.BottomLeft[1] - b },
    { rBg.BottomRight[0] - b, rBg.BottomRight[1] - b },
  };

  // bg
  color = colorLerp(colorButtonBg, 0, lerp);
	gfxScreenSpaceBox(&rBg, color, color, color, color);

  // fg
  color = colorLerp(colorButtonFg, 0, lerp);
	gfxScreenSpaceBox(&rFg, color, color, color, color);

  // draw name
  x = 0.5 * SCREEN_WIDTH;
  y = ((rFg.TopLeft[1] + rFg.BottomLeft[1]) * 440.0 * 0.5);
  gfxScreenSpaceText(x, y, 1, 1, colorLerp(colorText, 0, lerp), element->name, -1, 7);

  // add some padding
  rect->TopLeft[1] += 0.01;
  rect->TopRight[1] += 0.01;
}

//------------------------------------------------------------------------------
void mapsActionHandler(int elementId, int actionType, void * userdata)
{
  MenuElem_t* element = &menuElements[elementId];
  
  switch (actionType)
  {
    case ACTIONTYPE_SELECT:
    {
      configMenuDisable();
      if (mapsPromptEnableCustomMaps())
        element->enabled = 0;
      break;
    }
    case ACTIONTYPE_GETHEIGHT:
    {
      *(float*)userdata = lineHeight * 2;
      break;
    }
    case ACTIONTYPE_DRAW:
    {
      drawButtonMenuElement(element, (RECT*)userdata);
      break;
    }
  }
}

//------------------------------------------------------------------------------
void toggleActionHandler(int elementId, int actionType, void * userdata)
{
  MenuElem_t* element = &menuElements[elementId];

  switch (actionType)
  {
    case ACTIONTYPE_INCREMENT:
    case ACTIONTYPE_SELECT:
    case ACTIONTYPE_DECREMENT:
    {
      // toggle
      *(char*)element->userdata = !(*(char*)element->userdata);;
      break;
    }
    case ACTIONTYPE_GETHEIGHT:
    {
      *(float*)userdata = lineHeight;
      break;
    }
    case ACTIONTYPE_DRAW:
    {
      drawToggleMenuElement(element, (RECT*)userdata);
      break;
    }
  }
}

//------------------------------------------------------------------------------
void drawFrame(void)
{
  RECT r = {
    { rectBgBox.TopLeft[0], rectBgBox.TopLeft[1] },
    { rectBgBox.TopRight[0], rectBgBox.TopRight[1] },
    { rectBgBox.BottomLeft[0], rectBgBox.BottomLeft[1] },
    { rectBgBox.BottomRight[0], rectBgBox.BottomRight[1] }
  };

  // bg
	gfxScreenSpaceBox(&r, colorBg, colorBg, colorBg, colorBg);

  // title bg
  r.BottomRight[1] = r.TopLeft[1] + 0.075;
  r.BottomLeft[1] = r.TopLeft[1] + 0.075;
	gfxScreenSpaceBox(&r, colorRed, colorRed, colorRed, colorRed);

  // title
  gfxScreenSpaceText(0.5 * SCREEN_WIDTH, r.TopLeft[1] * SCREEN_HEIGHT, 1, 1, colorText, "Patch Config", -1, 1);

  // footer bg
  r.TopLeft[1] = rectBgBox.BottomLeft[1] - 0.075;
  r.TopRight[1] = rectBgBox.BottomLeft[1] - 0.075;
  r.BottomLeft[1] = rectBgBox.BottomLeft[1];
  r.BottomRight[1] = rectBgBox.BottomLeft[1];
	gfxScreenSpaceBox(&r, colorRed, colorRed, colorRed, colorRed);

  // footer
  gfxScreenSpaceText((r.BottomRight[0] * SCREEN_WIDTH) - 5, r.TopLeft[1] * SCREEN_HEIGHT, 1, 1, colorText, footerText, -1, 8);
}

//------------------------------------------------------------------------------
void onConfigOnlineMenu(void)
{
  int i = 0;
	const int menuElementsCount =  sizeof(menuElements) / sizeof(MenuElem_t);
  MenuElem_t* currentElement;
  RECT drawRect = {
    { rectBgBox.TopLeft[0], rectBgBox.TopLeft[1] + (lineHeight * 2) },
    { rectBgBox.TopRight[0], rectBgBox.TopRight[1] + (lineHeight * 2) },
    { rectBgBox.TopLeft[0], rectBgBox.TopLeft[1] + (lineHeight * 3) },
    { rectBgBox.TopRight[0], rectBgBox.TopRight[1] + (lineHeight * 3) }
  };

  if (isConfigMenuActive)
  {
    // prevent pad from affecting menus
    padDisableInput();

    // draw frame
    drawFrame();

    // draw items
    for (i = 0; i < menuElementsCount; ++i)
    {
      currentElement = &menuElements[i];
      float itemHeight = 0;
      currentElement->handler(i, ACTIONTYPE_GETHEIGHT, &itemHeight);

      // set rect to height
      drawRect.BottomLeft[1] = drawRect.TopLeft[1] + itemHeight;
      drawRect.BottomRight[1] = drawRect.TopRight[1] + itemHeight;

      // draw selection
      if (i == selectedMenuItem) {
        gfxScreenSpaceBox(&drawRect, colorSelected, colorSelected, colorSelected, colorSelected);
      }

      // draw
      currentElement->handler(i, ACTIONTYPE_DRAW, &drawRect);

      // increment rect
      drawRect.TopLeft[1] += itemHeight;
      drawRect.TopRight[1] += itemHeight;
    }

    // close
    if (padGetButtonUp(0, PAD_TRIANGLE) > 0)
    {
      configMenuDisable();
    }
    // nav down
    else if (padGetButtonDown(0, PAD_DOWN) > 0)
    {
      ++selectedMenuItem;
      if (selectedMenuItem >= menuElementsCount)
        selectedMenuItem = menuElementsCount - 1;
    }
    // nav up
    else if (padGetButtonDown(0, PAD_UP) > 0)
    {
      --selectedMenuItem;
      if (selectedMenuItem < 0)
        selectedMenuItem = 0;
    }
    // nav select
    else if (padGetButtonDown(0, PAD_CROSS) > 0)
    {
      if (menuElements[selectedMenuItem].enabled)
        menuElements[selectedMenuItem].handler(selectedMenuItem, ACTIONTYPE_SELECT, NULL);
    }
    // nav inc
    else if (padGetButtonDown(0, PAD_RIGHT) > 0)
    {
      if (menuElements[selectedMenuItem].enabled)
        menuElements[selectedMenuItem].handler(selectedMenuItem, ACTIONTYPE_DECREMENT, NULL);
    }
    // nav dec
    else if (padGetButtonDown(0, PAD_LEFT) > 0)
    {
      if (menuElements[selectedMenuItem].enabled)
        menuElements[selectedMenuItem].handler(selectedMenuItem, ACTIONTYPE_DECREMENT, NULL);
    }
  }
  else if (!mapsDownloadingModules())
  {
    if (uiGetActive() == UI_ID_ONLINE_MAIN_MENU)
    {
      // render message
      gfxScreenSpaceBox(&rectOpenBg, colorOpenBg, colorOpenBg, colorOpenBg, colorOpenBg);
      gfxScreenSpaceText(SCREEN_WIDTH * 0.3, SCREEN_HEIGHT * 0.7, 1, 1, 0x80FFFFFF, "\x1e Open Config Menu", -1, 1);
    }

		// check for pad input
		if (padGetButtonDown(0, PAD_SELECT) > 0)
		{
      configMenuEnable();
		}
  }
}

//------------------------------------------------------------------------------
void configMenuDisable(void)
{
  isConfigMenuActive = 0;

  // send config to server for saving
  netSendCustomAppMessage(netGetLobbyServerConnection(), CUSTOM_MSG_ID_CLIENT_USER_CONFIG, sizeof(PatchConfig_t), &config);

  // re-enable pad
  padEnableInput();
}

//------------------------------------------------------------------------------
void configMenuEnable(void)
{
  // enable
  isConfigMenuActive = 1;

  // ensure custom map installer button is only enabled if not yet tried to install
  menuElements[0].enabled = !mapsHasTriedLoading();
}
