#include <libdl/pad.h>
#include <libdl/ui.h>
#include <libdl/graphics.h>
#include <libdl/stdio.h>
#include <libdl/net.h>
#include <libdl/color.h>
#include <libdl/game.h>
#include "include/config.h"
#include "messageid.h"

#define LINE_HEIGHT         (0.05)
#define LINE_HEIGHT_3_2     (0.075)

enum ActionType
{
  ACTIONTYPE_DRAW,
  ACTIONTYPE_GETHEIGHT,
  ACTIONTYPE_SELECT,
  ACTIONTYPE_INCREMENT,
  ACTIONTYPE_DECREMENT
};

typedef void (*ActionHandler)(int elementId, int actionType, void * actionArg);
typedef void (*ButtonSelectHandler)(int elementId);

typedef struct MenuElem
{
  char name[32];
  int enabled;
  ActionHandler handler;
  void * userdata;
} MenuElem_t;

typedef struct MenuElem_ListData
{
  char * value;
  int count;
  char * items[];
} MenuElem_ListData_t;

// config
extern PatchConfig_t config;

// 
int isConfigMenuActive = 0;
int selectedMenuItem = 0;
int menuOffset = 0;
u32 padPointer = 0;

// constants
const char footerText[] = "\x14 HIDE     \x10 SELECT     \x12 BACK";

// menu display properties
const u32 colorBlack = 0x80000000;
const u32 colorBg = 0x80404040;
const u32 colorContentBg = 0x80202020;
const u32 colorRed = 0x80000040;
const u32 colorSelected = 0x80606060;
const u32 colorButtonBg = 0x80303030;
const u32 colorButtonFg = 0x80505050;
const u32 colorText = 0x80FFFFFF;
const u32 colorOpenBg = 0x20000000;

const float frameX = 0.1;
const float frameY = 0.15;
const float frameW = 0.8;
const float frameH = 0.7;
const float frameTitleH = 0.075;
const float frameFooterH = 0.05;
const float contentPaddingX = 0.01;
const float contentPaddingY = 0;

//
void configMenuDisable(void);
void configMenuEnable(void);

void buttonActionHandler(int elementId, int actionType, void * actionArg);
void toggleActionHandler(int elementId, int actionType, void * actionArg);
void listActionHandler(int elementId, int actionType, void * actionArg);

void mapsSelectHandler(int elementId);

int mapsHasTriedLoading(void);
int mapsPromptEnableCustomMaps(void);
int mapsDownloadingModules(void);

// level of detail list item
MenuElem_ListData_t dataLevelOfDetail = {
    &config.levelOfDetail,
    2,
    { "Low", "Normal", "High" }
};

// menu items
MenuElem_t menuElements[] = {
  { "Enable custom maps", 1, buttonActionHandler, mapsSelectHandler },
  { "Disable framelimiter", 1, toggleActionHandler, &config.disableFramelimiter },
  { "Announcers on all gamemodes", 1, toggleActionHandler, &config.enableGamemodeAnnouncements },
  { "Spectate mode", 1, toggleActionHandler, &config.enableSpectate },
  { "Singleplayer music", 0, toggleActionHandler, &config.enableSingleplayerMusic },
  { "Level of Detail", 1, listActionHandler, &dataLevelOfDetail },
};

// 
void mapsSelectHandler(int elementId)
{
  MenuElem_t* element = &menuElements[elementId];

  // 
  if (gameIsIn())
    return;

  // close menu
  configMenuDisable();

  // try and load the map
  // disable on success
  if (mapsPromptEnableCustomMaps())
    element->enabled = 0;
}

//------------------------------------------------------------------------------
void drawToggleMenuElement(MenuElem_t* element, RECT* rect)
{
  float x,y;
  float lerp = element->enabled ? 0.0 : 0.5;
  u32 color = colorLerp(colorText, 0, lerp);

  // draw name
  x = (rect->TopLeft[0] * SCREEN_WIDTH) + 5;
  y = (rect->TopLeft[1] * SCREEN_HEIGHT) + 5;
  gfxScreenSpaceText(x, y, 1, 1, color, element->name, -1, 0);

  // draw value
  x = (rect->TopRight[0] * SCREEN_WIDTH) - 5;
  gfxScreenSpaceText(x, y, 1, 1, color, *(char*)element->userdata ? "On" : "Off", -1, 2);
}

//------------------------------------------------------------------------------
void drawListMenuElement(MenuElem_t* element, MenuElem_ListData_t * listData, RECT* rect)
{
  float x,y;
  float lerp = element->enabled ? 0.0 : 0.5;
  u32 color = colorLerp(colorText, 0, lerp);

  // draw name
  x = (rect->TopLeft[0] * SCREEN_WIDTH) + 5;
  y = (rect->TopLeft[1] * SCREEN_HEIGHT) + 5;
  gfxScreenSpaceText(x, y, 1, 1, color, element->name, -1, 0);

  // draw value
  x = (rect->TopRight[0] * SCREEN_WIDTH) - 5;
  gfxScreenSpaceText(x, y, 1, 1, color, listData->items[(int)*listData->value], -1, 2);
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
	gfxScreenSpaceQuad(&rBg, color, color, color, color);

  // fg
  color = colorLerp(colorButtonFg, 0, lerp);
	gfxScreenSpaceQuad(&rFg, color, color, color, color);

  // draw name
  x = 0.5 * SCREEN_WIDTH;
  y = ((rFg.TopLeft[1] + rFg.BottomLeft[1]) * SCREEN_HEIGHT * 0.5);
  gfxScreenSpaceText(x, y, 1, 1, colorLerp(colorText, 0, lerp), element->name, -1, 4);

  // add some padding
  rect->TopLeft[1] += 0.01;
  rect->TopRight[1] += 0.01;
}

//------------------------------------------------------------------------------
void buttonActionHandler(int elementId, int actionType, void * actionArg)
{
  MenuElem_t* element = &menuElements[elementId];

  
  switch (actionType)
  {
    case ACTIONTYPE_SELECT:
    {
      if (element->userdata)
        ((ButtonSelectHandler)element->userdata)(elementId);
      break;
    }
    case ACTIONTYPE_GETHEIGHT:
    {
      *(float*)actionArg = LINE_HEIGHT * 2;
      break;
    }
    case ACTIONTYPE_DRAW:
    {
      drawButtonMenuElement(element, (RECT*)actionArg);
      break;
    }
  }
}

//------------------------------------------------------------------------------
void listActionHandler(int elementId, int actionType, void * actionArg)
{
  MenuElem_t* element = &menuElements[elementId];
  MenuElem_ListData_t* listData = (MenuElem_ListData_t*)element->userdata;
  int itemCount = listData->count;

  switch (actionType)
  {
    case ACTIONTYPE_INCREMENT:
    case ACTIONTYPE_SELECT:
    {
      char newValue = *listData->value + 1;
      if (newValue >= itemCount)
        newValue = 0;
      *listData->value = newValue;
      break;
    }
    case ACTIONTYPE_DECREMENT:
    {
      char newValue = *listData->value - 1;
      if (newValue < 0)
        newValue = itemCount - 1;
      *listData->value = newValue;
      break;
    }
    case ACTIONTYPE_GETHEIGHT:
    {
      *(float*)actionArg = LINE_HEIGHT;
      break;
    }
    case ACTIONTYPE_DRAW:
    {
      drawListMenuElement(element, listData, (RECT*)actionArg);
      break;
    }
  }
}
//------------------------------------------------------------------------------
void toggleActionHandler(int elementId, int actionType, void * actionArg)
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
      *(float*)actionArg = LINE_HEIGHT;
      break;
    }
    case ACTIONTYPE_DRAW:
    {
      drawToggleMenuElement(element, (RECT*)actionArg);
      break;
    }
  }
}

//------------------------------------------------------------------------------
void drawFrame(void)
{
  // bg
  gfxScreenSpaceBox(frameX, frameY, frameW, frameH, colorBg);

  // title bg
  gfxScreenSpaceBox(frameX, frameY, frameW, frameTitleH, colorRed);

  // title
  gfxScreenSpaceText(0.5 * SCREEN_WIDTH, (frameY + frameTitleH * 0.5) * SCREEN_HEIGHT, 1, 1, colorText, "Patch Config", -1, 4);

  // footer bg
  gfxScreenSpaceBox(frameX, frameY + frameH - frameFooterH, frameW, frameFooterH, colorRed);

  // footer
  gfxScreenSpaceText(((frameX + frameW) * SCREEN_WIDTH) - 5, (frameY + frameH) * SCREEN_HEIGHT - 5, 1, 1, colorText, footerText, -1, 8);

  // content bg
  gfxScreenSpaceBox(frameX + contentPaddingX, frameY + frameTitleH + contentPaddingY, frameW - (contentPaddingX*2), frameH - frameTitleH - frameFooterH - (contentPaddingY * 2), colorContentBg);
}

//------------------------------------------------------------------------------
void onUpdate(int inGame)
{
  int i = 0;
  int menuElementRenderEnd = menuOffset;
	const int menuElementsCount = sizeof(menuElements) / sizeof(MenuElem_t);
  MenuElem_t* currentElement;

  float contentX = frameX + contentPaddingX;
  float contentY = frameY + frameTitleH + contentPaddingY;
  float contentW = frameW - (contentPaddingX * 2);
  float contentH = frameH - frameTitleH - frameFooterH - (contentPaddingY * 2);
  RECT drawRect = {
    { contentX, contentY },
    { contentX + contentW, contentY },
    { contentX, contentY },
    { contentX + contentW, contentY }
  };

  if (isConfigMenuActive)
  {
    // prevent pad from affecting menus
    padDisableInput();

    // draw
    if (padGetButton(0, PAD_L1) <= 0)
    {
      // draw frame
      drawFrame();

      // draw items
      for (i = menuOffset; i < menuElementsCount; ++i)
      {
        currentElement = &menuElements[i];
        float itemHeight = 0;
        currentElement->handler(i, ACTIONTYPE_GETHEIGHT, &itemHeight);

        // ensure item is within content bounds
        if ((drawRect.BottomLeft[1] + itemHeight) > (contentY + contentH))
          break;

        // set rect to height
        drawRect.BottomLeft[1] = drawRect.TopLeft[1] + itemHeight;
        drawRect.BottomRight[1] = drawRect.TopRight[1] + itemHeight;

        // draw selection
        if (i == selectedMenuItem) {
          gfxScreenSpaceQuad(&drawRect, colorSelected, colorSelected, colorSelected, colorSelected);
        }

        // draw
        currentElement->handler(i, ACTIONTYPE_DRAW, &drawRect);

        // increment rect
        drawRect.TopLeft[1] += itemHeight;
        drawRect.TopRight[1] += itemHeight;

        menuElementRenderEnd = i + 1;
      }
      
      // draw scroll bar
      if (menuOffset > 0 || menuElementRenderEnd < menuElementsCount)
      {
        float scrollValue = menuOffset / (float)(menuElementsCount - (menuElementRenderEnd-menuOffset));
        float scrollBarHeight = 0.05;
        float contentRectHeight = contentH - scrollBarHeight;

        gfxScreenSpaceBox(contentX + contentW, contentY + (scrollValue * contentRectHeight), 0.01, scrollBarHeight, colorRed);
      }

      // 
      if (selectedMenuItem >= menuElementRenderEnd)
        ++menuOffset;
      if (selectedMenuItem < menuOffset)
        menuOffset = selectedMenuItem;
    }

    // 
    currentElement = &menuElements[selectedMenuItem];

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
      if (currentElement->enabled)
        currentElement->handler(selectedMenuItem, ACTIONTYPE_SELECT, NULL);
    }
    // nav inc
    else if (padGetButtonDown(0, PAD_RIGHT) > 0)
    {
      if (currentElement->enabled)
        currentElement->handler(selectedMenuItem, ACTIONTYPE_INCREMENT, NULL);
    }
    // nav dec
    else if (padGetButtonDown(0, PAD_LEFT) > 0)
    {
      if (currentElement->enabled)
        currentElement->handler(selectedMenuItem, ACTIONTYPE_DECREMENT, NULL);
    }
  }
  else if (!inGame && !mapsDownloadingModules())
  {
    if (uiGetActive() == UI_ID_ONLINE_MAIN_MENU)
    {
      // render message
      gfxScreenSpaceBox(0.1, 0.75, 0.4, 0.05, colorOpenBg);
      gfxScreenSpaceText(SCREEN_WIDTH * 0.3, SCREEN_HEIGHT * 0.775, 1, 1, 0x80FFFFFF, "\x1e Open Config Menu", -1, 4);
    }

		// check for pad input
		if (padGetButtonDown(0, PAD_SELECT) > 0)
		{
      configMenuEnable();
		}
  }
}

//------------------------------------------------------------------------------
void onConfigGameMenu(void)
{
  onUpdate(1);
}

//------------------------------------------------------------------------------
void onConfigOnlineMenu(void)
{
  onUpdate(0);
}

//------------------------------------------------------------------------------
void configMenuDisable(void)
{
  if (!isConfigMenuActive)
    return;
  
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
