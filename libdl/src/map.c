#include "map.h"

//--------------------------------------------------------
int mapMaskHasMask(int value, int maskValue)
{
    return (value & maskValue) == maskValue;
}

//--------------------------------------------------------
int mapIdToMask(int mapId)
{
    switch (mapId)
    {
        case MAP_ID_BATTLEDOME: return MAP_MASK_BATTLEDOME;
        case MAP_ID_CATACROM: return MAP_MASK_CATACROM;
        case MAP_ID_SARATHOS: return MAP_MASK_SARATHOS;
        case MAP_ID_DC: return MAP_MASK_DC;
        case MAP_ID_SHAAR: return MAP_MASK_SHAAR;
        case MAP_ID_TORVAL: return MAP_MASK_TORVAL;
        case MAP_ID_VALIX: return MAP_MASK_VALIX;
        case MAP_ID_TEMPUS: return MAP_MASK_TEMPUS;
        case MAP_ID_MF: return MAP_MASK_MF;
        case MAP_ID_MARAXUS: return MAP_MASK_MARAXUS;
        case MAP_ID_GS: return MAP_MASK_GS;
        default: return 0;
    }
}

//--------------------------------------------------------
int mapMaskToId(int mapMask)
{
    switch (mapMask)
    {
        case MAP_MASK_BATTLEDOME: return MAP_ID_BATTLEDOME;
        case MAP_MASK_CATACROM: return MAP_ID_CATACROM;
        case MAP_MASK_SARATHOS: return MAP_ID_SARATHOS;
        case MAP_MASK_DC: return MAP_ID_DC;
        case MAP_MASK_SHAAR: return MAP_ID_SHAAR;
        case MAP_MASK_TORVAL: return MAP_ID_TORVAL;
        case MAP_MASK_VALIX: return MAP_ID_VALIX;
        case MAP_MASK_TEMPUS: return MAP_ID_TEMPUS;
        case MAP_MASK_MF: return MAP_ID_MF;
        case MAP_MASK_MARAXUS: return MAP_ID_MARAXUS;
        case MAP_MASK_GS: return MAP_ID_GS;
        default: return 0;
    }
}
