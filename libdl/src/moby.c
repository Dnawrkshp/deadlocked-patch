#include "moby.h"

//--------------------------------------------------------
#define MOBY_ARRAY                              ((Moby**)0x0026BDA0)
#define SPAWN_MOBY_FUNC                         (0x004F7200)
#define WATER_MOBY_PTR                          ((Moby**)0x0022D470)
#define MOBY_UPDATE_FUNC                        (0x004FE720)
#define MOBY_DESTROY_FUNC                       (0x004F76A0)

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

/*
 * Destroys the moby.
 */
void mobyDestroy(Moby * moby)
{
    if (!moby)
        return;

    ((void (*)(Moby*))MOBY_DESTROY_FUNC)(moby);
}

/*
 * Updates the moby's transform
 */
void mobyUpdateTransform(Moby * moby)
{
    if (!moby)
        return;

    ((void (*)(Moby*))MOBY_UPDATE_FUNC)(moby);
}
