////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// GameView.c: GameView ADT implementation
//
// 2014-07-01    v1.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01    v1.1    Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31    v2.0    Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10    v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sysexits.h>

#include "main_game_view.h"
#include "GameView.h"
// #include "map.h" ... if you decide to use the Map ADT

typedef struct gameView {
    MainGameView gv;
} gameView;

// create new GameView
GameView GvNew (char *pastPlays, Message messages[])
{
    GameView new = malloc (sizeof *new);
    if (new == NULL) {
        fprintf (stderr, "Couldn't allocate GameView!\n");
        exit (EXIT_FAILURE);
    }
    new->gv = main_gv_new (pastPlays, messages, true);
    return new;
}

// delete created DraculaView
void GvFree (GameView gv)
{
    main_gv_drop (gv->gv);
    free (gv);
}

// obtain current round No.
Round GvGetRound (GameView gv)
{
    return main_gv_get_round (gv->gv);
}

// obtain current player No. (enum)
Player GvGetPlayer (GameView gv)
{
    return main_gv_get_player (gv->gv);
}

// Get current game score
int GvGetScore (GameView gv)
{
    return main_gv_get_score (gv->gv);
}

// Get selected player's health
int GvGetHealth (GameView gv, Player player)
{
    return main_gv_get_health (gv->gv, player);
}

// Get selected player's location
PlaceId GvGetLocation (GameView gv, Player player)
{
    return main_gv_get_location (gv->gv, player);
}

// Get selected player's location
PlaceId GvGetPlayerLocation (GameView gv, Player player)
{
    return GvGetLocation (gv, player);
}

// Get Vampire location
PlaceId GvGetVampireLocation (GameView gv)
{
    return main_gv_get_vampire_location (gv->gv);
}

// Get Trap locations
PlaceId *GvGetTrapLocations (GameView gv, int *numTraps) 
{
    return main_gv_get_trap_locations (gv->gv, numTraps);
}

// Get location history of selected player for selected length
void GvGetTrailHistory (GameView gv, Player player, PlaceId trail[TRAIL_SIZE])
{
    main_gv_get_move_history (gv->gv, player, trail);
}

// Get all move history of selected player        
PlaceId *GvGetMoveHistory (GameView gv, Player player,
                            int *numReturnedMoves, bool *canFree)
{
    *canFree = false;
    return main_gv_get_all_move (gv->gv, player, numReturnedMoves);
}

// Get all location history of selected player 
PlaceId *GvGetLocationHistory (GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
    *canFree = false;
    return main_gv_get_all_location (gv->gv, player, numReturnedLocs);
}          

// Get last location of selected player
PlaceId *GvGetLastLocations (GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
    *canFree = true;
    return main_gv_get_last_location (gv->gv, player, numLocs, numReturnedLocs);
}

// Get last move of selected player
PlaceId *GvGetLastMoves (GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{
    *canFree = true;
    return main_gv_get_last_move (gv->gv, player, numMoves, numReturnedMoves);
}                 

// Get reachable location of player
PlaceId *GvGetReachable (GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
    return main_gv_player_get_connections (gv->gv->players[player], numReturnedLocs, from,
                                player, round, true, true, true, false, true, false);
}    

// Get reachable location of player
PlaceId *GvGetReachableByType (GameView gv, Player player, Round round,
                                PlaceId from, bool road, bool rail,
                                bool boat, int *numReturnedLocs)
{
    return main_gv_player_get_connections (gv->gv->players[player], numReturnedLocs, from,
                                player, round, road, rail, boat, false, true, false);
}  

// Get connections from selected location
PlaceId *GvGetConnections (
    GameView gv, int *n_locations,
    PlaceId from, Player player, Round round,
    bool road, bool rail, bool sea)
{ 
    return main_gv_player_get_connections (gv->gv->players[player], n_locations, from,
                                 player, round, road, rail, sea, false, true, false);

}
