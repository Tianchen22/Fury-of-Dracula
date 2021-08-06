////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.c: the DraculaView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sysexits.h>

#include "DraculaView.h"
#include "Game.h"
#include "main_game_view.h"
#include "Map.h"
#include "Mapdata.h"

typedef struct draculaView {
    MainGameView gv;
} draculaView;

// create new DraculaView
DraculaView DvNew (char *pastPlays, Message messages[])
{
    /// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    DraculaView new = malloc (sizeof *new);
    if (new == NULL) err (EX_OSERR, "couldn't allocate DraculaView");
    new->gv = main_gv_new (pastPlays, messages, true);
    return new;
}

// delete created DraculaView
void DvFree (DraculaView dv)
{
    main_gv_drop (dv->gv);
    free (dv);
}

// obtain current round No.
// (regardless player played or not in current round)
Round DvGetRound (DraculaView dv)
{
    return main_gv_get_round (dv->gv);
}

// Get current game score
int DvGetScore (DraculaView dv)
{
    return main_gv_get_score (dv->gv);
}

// Get selected player's health
int DvGetHealth (DraculaView dv, Player player)
{
    return main_gv_get_health (dv->gv, player);
}

// Get selected player's location
PlaceId DvGetPlayerLocation (DraculaView dv, Player player)
{
    return main_gv_get_location (dv->gv, player);
}

// Get Vampire location
PlaceId DvGetVampireLocation (DraculaView dv) 
{
    return main_gv_get_vampire_location (dv->gv);
}

// Get Trap locations
PlaceId *DvGetTrapLocations (DraculaView dv, int *numTraps)
{
    return main_gv_get_trap_locations (dv->gv, numTraps);
}

// Get valid Dracula *moves* next
PlaceId *DvGetValidMoves (DraculaView dv, int *numReturnedMoves)
{
    printf ("%d\n", *numReturnedMoves);    
    
    if (DvGetRound (dv) == 0) {
    *numReturnedMoves = 70;
    PlaceId *ret = malloc (70 * sizeof (PlaceId));
        for (int i = MIN_REAL_PLACE, j = 0; i <= MAX_REAL_PLACE; i++, j++) {
            if (i == HOSPITAL_PLACE) {
                j--;
                continue;
            }
            ret[j] = i;
        }
        return ret;
    }
    return main_gv_player_get_connections (
        main_gv_get_player_class (dv->gv, PLAYER_DRACULA), numReturnedMoves,
        DvGetPlayerLocation (dv, PLAYER_DRACULA), PLAYER_DRACULA, DvGetRound (dv),
        true, false, true, true, false, true);
}

// Get valid Dracula *locations* next
PlaceId *DvWhereCanIGo (DraculaView dv, int *numReturnedLocs)
{
    return main_gv_player_get_connections (
                                dv->gv->players[PLAYER_DRACULA], numReturnedLocs, 
                                DvGetPlayerLocation (dv, PLAYER_DRACULA), PLAYER_DRACULA,
                                DvGetRound (dv), true, false, true, true, false, true);
}

// Returns an array of size (int *numReturnedLocs)
// Storing where player can go next 
PlaceId *DvWhereCanIGoByType (DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
    return main_gv_player_get_connections (
                                dv->gv->players[PLAYER_DRACULA], numReturnedLocs, 
                                DvGetPlayerLocation (dv, PLAYER_DRACULA), PLAYER_DRACULA,
                                DvGetRound (dv), road, false, boat, true, true, true);
}

// Returns an array of size (int *numReturnedLocs)
// Storing where player can go next
PlaceId *DvWhereCanTheyGo (DraculaView dv, Player player,
                          int *numReturnedLocs)
{
    if (player == PLAYER_DRACULA) return DvWhereCanIGo (dv, numReturnedLocs);

	// round fix;
    Round round = -1;
    if (dv->gv->players[player]->all_history_size == DvGetRound (dv)){
        round = DvGetRound (dv);
    } else {
        round = DvGetRound (dv) + 1;
    }
	return main_gv_player_get_connections (
                                dv->gv->players[player], numReturnedLocs, 
                                DvGetPlayerLocation (dv, player), player,
                                round, true, true, true, false, true, false);
}

// Returns an array of size (int *numReturnedLocs)
// Storing where player can go next based on selected transport type
PlaceId *DvWhereCanTheyGoByType (DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
    if (player == PLAYER_DRACULA) return DvWhereCanIGoByType (dv, road, boat, numReturnedLocs);

	// round fix;
    Round round = -1;
    if (dv->gv->players[player]->all_history_size == DvGetRound (dv)){
        round = DvGetRound (dv);
    } else {
        round = DvGetRound (dv) + 1;
    }
	return main_gv_player_get_connections (
                                dv->gv->players[player], numReturnedLocs, 
                                DvGetPlayerLocation (dv, player), player,
                                round, road, rail, boat, false, true, false);
}

// Get player's move
void DvGetPlayerMove (
    DraculaView dv, Player player,
    PlaceId *start, PlaceId *end)
{
    PlaceId trail[TRAIL_SIZE];
	main_gv_get_move_history (dv->gv, player, trail);

	*end = trail[0];
	*start = trail[1];
}

// Get information of encounter of selected place
void DvGetLocaleInfo (
    DraculaView dv, PlaceId where,
    int *n_traps, int *n_vamps)
{
    main_gv_get_locale_info (dv->gv, where, n_traps, n_vamps);
}

// Get thr trail of a player
void DvGetTrail (
    DraculaView dv, Player player,
    PlaceId trail[TRAIL_SIZE])
{
    if (player == PLAYER_DRACULA)
        main_gv_get_location_history (dv->gv, player, trail);
    else
        main_gv_get_move_history (dv->gv, player, trail);
}

// Get possible destinations of Dracula
PlaceId *DvGetDests (DraculaView dv, int *n_locations, bool road,
                         bool sea)
{
    return main_gv_player_get_connections (
                                dv->gv->players[PLAYER_DRACULA], n_locations, 
                                DvGetPlayerLocation (dv, PLAYER_DRACULA), PLAYER_DRACULA,
                                DvGetRound (dv), road, false, sea, true, true, false);
}

// Get possible destinations of player
PlaceId *DvGetDestsPlayer (
    DraculaView dv, int *n_locations, Player player,
    bool road, bool rail, bool sea)
{
    if (player == PLAYER_DRACULA) return DvGetDests (dv, n_locations, road, sea);
      return main_gv_player_get_connections (
                                dv->gv->players[player], n_locations, 
                                DvGetPlayerLocation (dv, player), player,
                                DvGetRound (dv), road, rail, sea, false, true, false);
}
