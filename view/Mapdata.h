#include <stdbool.h>
#include <stdlib.h>
#include "Places.h"

#ifndef FOD__MAPDATA_H_
#define FOD__MAPDATA_H_

#define MAX_CONNECTION_PER_PLACE 10
extern char ADJMATRIX[][NUM_REAL_PLACES];
extern int SPDIST[][NUM_REAL_PLACES];
extern int SPROUND[][NUM_REAL_PLACES][4];

extern int ADJLIST_COUNT[];

extern struct adj_connection {
    PlaceId v;
    TransportType type;
} ADJLIST[][MAX_CONNECTION_PER_PLACE];

#endif  // !defined (FOD__MAPDATA_H_)
