#include "moby.h"

//--------------------------------------------------------
#define MOBY_ARRAY                              ((Moby**)0x0026BDA0)
#define SPAWN_MOBY_FUNC                         (0x004F7200)
#define WATER_MOBY_PTR                          ((Moby**)0x0022D470)


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

/*
 * Returns the water moby if it exists.
 */
Moby * getWaterMoby(void)
{
    return *WATER_MOBY_PTR;
}
