#include "moby.h"

//--------------------------------------------------------
#define MOBY_ARRAY                              ((Moby**)0x0026BDA0)
#define SPAWN_MOBY_FUNC                         (0x004F7200)


/*
 * Returns the collection of pointers to each loaded moby.
 */
Moby ** getLoadedMobies(void)
{
    return MOBY_ARRAY;
}

/*
 * Spawns a moby with the given id and properties size.
 */
Moby * spawnMoby(int id, int propSize)
{
    return ((Moby * (*)(int, int))SPAWN_MOBY_FUNC)(id, propSize);
}
