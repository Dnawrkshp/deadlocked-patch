#ifndef __PATCH_CONFIG_H__
#define __PATCH_CONFIG_H__

enum ActionType
{
  ACTIONTYPE_DRAW,
  ACTIONTYPE_GETHEIGHT,
  ACTIONTYPE_SELECT,
  ACTIONTYPE_INCREMENT,
  ACTIONTYPE_DECREMENT
};

enum ElementState
{
  ELEMENT_HIDDEN,
  ELEMENT_DISABLED,
  ELEMENT_ENABLED
};

struct MenuElem;
struct TabElem;

typedef void (*ActionHandler)(struct TabElem* tab, struct MenuElem* element, int actionType, void * actionArg);
typedef void (*ButtonSelectHandler)(struct TabElem* tab, struct MenuElem* element);
typedef void (*TabStateHandler)(struct TabElem* tab, int * state);

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

typedef struct TabElem
{
  char name[32];
  TabStateHandler stateHandler;
  MenuElem_t * elements;
  int elementsCount;
} TabElem_t;

#endif // __PATCH_CONFIG_H__
