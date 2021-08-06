#include <stdbool.h>

#include "Mapdata.h"
#include "Places.h"

#ifndef FOD__MAP_H_
#define FOD__MAP_H_

char get_connection_type_mask(TransportType t);
bool is_connected_via(PlaceId v, PlaceId w, char tspt);
bool is_connected(PlaceId v, PlaceId w);
struct adj_connection *get_connections(PlaceId v);
PlaceId special_location_find_by_abbrev(char *abbrev);

#endif  // !defined (FOD__MAP_H_)
