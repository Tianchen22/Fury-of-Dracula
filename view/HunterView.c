////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// HunterView.c: the HunterView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sysexits.h>

#include "HunterView.h"
#include "Game.h"
#include "main_game_view.h"
#include "Map.h"
#include "Mapdata.h"

typedef struct hunterView
{
    MainGameView gv;
} hunterView;

// create new HunterView
HunterView HvNew (char *past_plays, Message messages[])
{

    HunterView new = malloc (sizeof *new);
    if (new == NULL) err (EX_OSERR, "couldn't allocate DraculaView");
    new->gv = main_gv_new (past_plays, messages, true);
    return new;
}

// delete created HunterView
void HvFree (HunterView hv)
{
    main_gv_drop (hv->gv);
    free (hv);
}

// obtain current round No.
// (regardless player played or not in current round)
Round HvGetRound (HunterView hv)
{
    return main_gv_get_round (hv->gv);
}

// obtain current player No. (enum)
Player HvGetPlayer (HunterView hv)
{
    return main_gv_get_player (hv->gv);
}

// Get current game score
int HvGetScore (HunterView hv)
{
    return main_gv_get_score (hv->gv);
}

// Get selected player's health
int HvGetHealth (HunterView hv, Player player)
{
    return main_gv_get_health (hv->gv, player);
}

// Get selected player's location
PlaceId HvGetPlayerLocation (HunterView hv, Player player)
{
    return main_gv_get_location (hv->gv, player);
}

// Get Vampire location
PlaceId HvGetVampireLocation (HunterView hv)
{
    return main_gv_get_vampire_location (hv->gv);
}

// Utility Functions
// Get last known Dracula location for chasing purpose
PlaceId HvGetLastKnownDraculaLocation (HunterView hv, Round *round)
{
    return main_gv_last_known_dracula_loc (hv->gv, round);
}

// Get one of the shortest path to selected location
// Path length of shortest path is stored in int *pathLength
PlaceId *HvGetShortestPathTo (HunterView hv, Player hunter,
                              PlaceId dest, int *pathLength)
{
    *pathLength = 0;
    return main_gv_hunter_shortest_path (
                    hv->gv, hunter, 
                    dest, pathLength);
}

// Returns an array of size (int *numReturnedLocs)
// Storing where player can go next 
// (next round if already played in current round, otherwise current round)
PlaceId *HvWhereCanIGo (HunterView hv, int *numReturnedLocs)
{
    Player curr = hv->gv->current_player;

    // round fix;
    Round round = -1;
    if (hv->gv->players[curr]->all_history_size == HvGetRound (hv)){
        round = HvGetRound (hv);
    } else {
        round = HvGetRound (hv) + 1;
    }

    return main_gv_player_get_connections (
                hv->gv->players[curr], numReturnedLocs,
                HvGetPlayerLocation (hv, curr), curr,
                round, true, true, 
                true, true, true, false);
}

// Returns an array of size (int *numReturnedLocs)
// Storing where player can go next based on selected transport type
// (next round if already played in current round, otherwise current round)
PlaceId *HvWhereCanIGoByType (HunterView hv, bool road, bool rail,
                             bool boat, int *numReturnedLocs)
{
    Player curr = hv->gv->current_player;

    // round fix;
    Round round = -1;
    if (hv->gv->players[curr]->all_history_size == HvGetRound (hv)){
        round = HvGetRound (hv);
    } else {
        round = HvGetRound (hv) + 1;
    }

    return main_gv_player_get_connections (
                hv->gv->players[curr], numReturnedLocs, 
                HvGetPlayerLocation (hv, curr), curr,
                round, road, rail, 
                boat, true, true, false);
}

// Returns an array of size (int *numReturnedLocs)
// Storing where other selected player can go next 
// (next round if already played in current round, otherwise current round)
PlaceId *HvWhereCanTheyGo (HunterView hv, Player player,
                          int *numReturnedLocs)
{
    // round fix;
    Round round = -1;
    if (hv->gv->players[player]->all_history_size == HvGetRound (hv)){
        round = HvGetRound (hv);
    } else {
        round = HvGetRound (hv) + 1;
    }

    return main_gv_player_get_connections (
                hv->gv->players[player], numReturnedLocs, 
                HvGetPlayerLocation (hv, player), player,
                round, true, true, 
                true, true, true, false);
}


// Returns an array of size (int *numReturnedLocs)
// Storing where other selected player can go next based on selected transport type
// (next round if already played in current round, otherwise current round)
PlaceId *HvWhereCanTheyGoByType (HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
    // *numReturnedLocs = 0;
    Round round = -1;
    if (hv->gv->players[player]->all_history_size == HvGetRound (hv)){
        round = HvGetRound (hv);
    } else {
        round = HvGetRound (hv) + 1;
    }

    return main_gv_player_get_connections (
                hv->gv->players[player], numReturnedLocs, 
                HvGetPlayerLocation (hv, player), player,
                round, road, rail, 
                boat, true, true, false);
}

// Returns Hunter trail
void HvGetTrail (
    HunterView hv, Player player,
    PlaceId trail[TRAIL_SIZE])
{
    main_gv_get_move_history (hv->gv, player, trail);
}

// Returns Hunter's destination
PlaceId *HvGetDests (
    HunterView hv, int *n_locations,
    bool road, bool rail, bool sea)
{
    return HvGetDestsPlayer (hv, n_locations, HvGetPlayer (hv), road, rail,
                             sea);
}

// Returns other Hunter's destination
PlaceId *HvGetDestsPlayer (
    HunterView hv, int *n_locations, Player player,
    bool road, bool rail, bool sea)
{
  if (player == PLAYER_DRACULA) {
    return main_gv_player_get_connections (hv->gv->players[player], n_locations, HvGetPlayerLocation (hv, player), player,
                                HvGetRound (hv), road, rail, sea, true, true, false);
  }
  return main_gv_player_get_connections (hv->gv->players[player], n_locations, HvGetPlayerLocation (hv, player), player,
                                HvGetRound (hv), road, rail, sea, false, true, false);
}
