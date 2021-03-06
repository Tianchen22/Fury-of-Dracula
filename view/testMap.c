////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// testMap.c: a simple program that checks the Map ADT
// You can change this as much as you want!
// You do not need to submit this file.
//
// 2020-07-10	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Map.h"
#include "Places.h"

#define MAX_LINE 1024

int main(void)
{	
	char buffer[MAX_LINE];
	
	printf("\nType a location name to see its connections: ");
	while (fgets(buffer, MAX_LINE, stdin) != NULL) {
		buffer[strlen(buffer) - 1] = '\0'; // remove newline character
		
		PlaceId place = placeNameToId(buffer);
		if (place == NOWHERE) {
			printf("Unknown location '%s'\n", buffer);
		} else {
			struct adj_connection *map = get_connections(place); 
			for (int i = 0; i < MAX_CONNECTION_PER_PLACE && is_connected(map[i].v,place); i++){
				struct adj_connection c = map[i];
				const char *dest = placeIdToName(c.v);
				const char *transportType = transportTypeToString(c.type);
				assert(is_connected(c.v, place) == true);
				printf("%s connects to %s by %s\n", buffer, dest, transportType);
			}
		}
		
		printf("\nType a location name to see its connections: ");
	}
}

//////////////////////////////////////////////////////////////////
// test if needed for the following function
// bool is_connected_via(PlaceId v, PlaceId w, char tspt);
// bool is_connected(PlaceId v, PlaceId w);
