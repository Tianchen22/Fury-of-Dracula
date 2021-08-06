#include <assert.h>

#include "Map.h"
#include "Mapdata.h"

char get_connection_type_mask(TransportType t) 
{
 assert(t >= MIN_TRANSPORT && t <= MAX_TRANSPORT);
    switch (t) {
        case ROAD:
            return 1;  // 001
        case RAIL:
            return 2;  // 010
        case BOAT:
            return 4;  // 100
        default:
            __builtin_unreachable();
    }
    __builtin_unreachable();
}

bool is_connected_via(PlaceId v, PlaceId w, char tspt) 
{
    return ((ADJMATRIX[v][w] & tspt) > 0);  
}

bool is_connected(PlaceId v, PlaceId w) 
{
    return is_connected_via(v, w, 7);  // 111
}

struct adj_connection *get_connections(PlaceId v) 
{
    return ADJLIST[v];
}

PlaceId special_location_find_by_abbrev(char *abbrev) 
{
    if (abbrev[0] == 'C' && abbrev[1] == '?') return CITY_UNKNOWN;
    if (abbrev[0] == 'S' && abbrev[1] == '?') return SEA_UNKNOWN;
    if (abbrev[0] == 'H' && abbrev[1] == 'I') return HIDE;
    if (abbrev[0] == 'T' && abbrev[1] == 'P') return TELEPORT;
    if (abbrev[0] == 'D') {
        if (abbrev[1] >= '1' && abbrev[1] <= '5')
            return DOUBLE_BACK_1 + abbrev[1] - '1';
        return NOWHERE;
    }
    return NOWHERE;
}
