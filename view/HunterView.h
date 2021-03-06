////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// HunterView.h: the HunterView ADT
//
// You may augment this with as many of your own functions as you like,
// but do NOT remove or modify any of the existing function signatures,
// otherwise, your code will not be able to compile with our tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-11-30	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#ifndef FOD__HUNTER_VIEW_H_
#define FOD__HUNTER_VIEW_H_

#include <stdbool.h>

#include "Game.h"
#include "GameView.h"
#include "Places.h"

typedef struct hunterView *HunterView;

/**
 * Creates a new view to summarise the current state of the game.
 *
 * @param past_plays is a string of all plays made in the game so far by
 *    all players (including Dracula) from earliest to most recent.
 *
 * @param messages is an array containing a `player_message` for each
 *    play in the game so far.  It will have exactly the same number of
 *    elements as there are plays in `past_plays`.  The message from the
 *    first play will be at index 0, and so on.  The contents of each
 *    `player_message` will be exactly as provided by the player.
 *
 * The "player_message" type is defined in game.h.
 * You are free to ignore messages if you wish.
 */
HunterView HvNew(char *past_plays, Message messages[]);

/**
 * Frees all resources allocated for `hv`.
 * After this has been called, `hv` should not be accessed.
 */
void HvFree(HunterView hv);

/**
 * Get the current round.
 */
Round HvGetRound(HunterView hv);

/**
 * Get the current player; effectively, whose turn is it?
 */
Player HvGetPlayer(HunterView hv);

/**
 * Get the current score, a positive integer between 0 and 366.
 */
int HvGetScore(HunterView hv);

/**
 * Get the current health points for a given player.
 * @param player specifies which players's life/blood points to return.
 * @returns a value between 0..9 for Hunters, or >0 for Dracula
 */
int HvGetHealth(HunterView hv, Player player);

/**
 * Get the current location of a given player.
 *
 * May be `UNKNOWN_LOCATION` if the player has not had a turn yet
 * (i.e., at the beginning of the game when the round is 0)
 *
 * Possible values include:
 * - in the interval 0..70, if the player was (known to be) in a
 *   particular city or on a particular sea;
 * - `CITY_UNKNOWN`, if Dracula was known to be in a city;
 * - `SEA_UNKNOWN`, if Dracula was known to be at sea;
 * - `HIDE`, if Dracula was known to have made a hide move;
 * - `DOUBLE_BACK_n`, where n is [1...5], if Dracula was known to have
 *   made a double back move _n_ positions back in the trail; e.g.,
 *   `DOUBLE_BACK_1` is the last place place he visited; or
 * - `TELEPORT`, if Dracula apparated back to Castle Dracula.
 */
PlaceId HvGetPlayerLocation(HunterView hv, Player player);

/**
 * Gets the location of the sleeping immature vampire.
 *
 * This function should return:
 * - NOWHERE  if the immature vampire does not exist (i.e., if it hasn't
 *   been spawned, or if it has already matured or been vanquished).
 * - A PlaceId corresponding to a real location, if the location of  the
 *   immature  vampire  has been revealed in the play string. This means
 *   Dracula's location during the round in which he placed an  immature
 *   vampire was revealed.
 * - Otherwise,  CITY_UNKNOWN.  Note that this function can never return
 *   SEA_UNKNOWN, since Dracula never places encounters at sea.
 */
PlaceId HvGetVampireLocation(HunterView hv);

////////////////////////////////////////////////////////////////////////
// Utility Functions

/**
 * Gets  Dracula's  last  known  real  location  as revealed in the play
 * string and sets *round to the number of the  latest  round  in  which
 * Dracula moved there.
 *
 * If Dracula's location has never been reveled in the play string, this
 * function should return NOWHERE and leave *round as it is.
 */
PlaceId HvGetLastKnownDraculaLocation(HunterView hv, Round *round);

/**
 * Gets  the  shortest  path from the given hunter's current location to
 * the given location, taking into account all connection types and  the
 * fact that hunters can only move once per round.
 *
 * This function should store the path in a dynamically allocated array,
 * and set *pathLength to the length of the path. If there are  multiple
 * shortest  paths, any of them can be returned. The path should exclude
 * the hunter's current location.
 *
 * EXAMPLE: If  the  hunter  is currently at Cadiz (CA) and the shortest
 *          path to Barcelona (BA) was Cadiz -> Madrid -> Barcelona, the
 *          returned array should contain just [Madrid,  Barcelona]  and
 *          *pathLength should be set to 2.
 *
 * NOTE: Since  this function will require a traversal of the map, which
 *       is a lot of work for just a single path, you may  want to store
 *       the  result of the traversal in your HunterView data structure,
 *       so that future calls to this function with the same player will
 *       be less expensive.
 */
PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest,
                             int *pathLength);

////////////////////////////////////////////////////////////////////////
// Making a Move

/**
 * Gets  all  the  locations  that the current hunter player can move to
 * this turn.
 *
 * This  function should return the locations in a dynamically allocated
 * array, and set *numReturnedLocs to the number of locations  returned.
 * The  array  can  be in any order but must contain unique entries. The
 * returned array should include the player's current location.
 *
 * If the current player hasn't made a move yet, set *numReturnedLocs to
 * 0 and return NULL.
 */
PlaceId *HvWhereCanIGo(HunterView hv, int *numReturnedLocs);

/**
 * Similar  to  HvWhereCanIGo, but the caller can restrict the transport
 * types to be considered. For example, if road and rail are  true,  but
 * boat is false, boat connections will be ignored.
 */
PlaceId *HvWhereCanIGoByType(HunterView hv, bool road, bool rail,
                             bool boat, int *numReturnedLocs);

/**
 * Gets  all  the  locations  that the given player can move to in their
 * next move (for the current player that is this turn).
 *
 * This  function should return the locations in a dynamically allocated
 * array, and set *numReturnedLocs to the number of locations  returned.
 * The  array  can  be in any order but must contain unique entries. The
 * returned array should include the player's current location.
 *
 * If  the given player is a hunter, the function must take into account
 * the round number of the player's next move for  determining  how  far
 * the player can travel by rail.
 *
 * If  the  given player is Dracula, the function must take into account
 * the fact that Dracula may not travel by rail or move to the  Hospital
 * of  St  Joseph  and  St  Mary. But it shouldn't take into account the
 * trail restriction. If Dracula's current location is not revealed, the
 * function should set *numReturnedLocs to 0 and return NULL.
 *
 * If the given player hasn't made a move yet, set *numReturnedLocs to 0
 * and return NULL.
 */
PlaceId *HvWhereCanTheyGo(HunterView hv, Player player,
                          int *numReturnedLocs);

/**
 * Similar to HvWhereCanTheyGo but the caller can restrict the transport
 * types  to  be considered. For example, if road and rail are true, but
 * boat is false, boat connections will be ignored.
 */
PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs);


//------------------------------------------------------------
/**
 * Fills the trail array with the locations of the last 6 turns for the
 * given player.
 *
 * If the move does not exist yet (i.e., the start of the game),
 * the value should be UNKNOWN_LOCATION (-1).
 *
 * For example after 2 turns the array may have the contents
 *
 *     {29, 12, -1, -1, -1, -1}
 *
 * This would mean in the first move the player started on location 12
 * then moved to the current location of 29.
 *
 * Hunters can only get approximate information about where Dracula has
 * been (e.g. `SEA_UNKNOWN`, etc.), unless Dracula is at Castle Dracula,
 * or has been encountered
 */
void HvGetTrail (
    HunterView hv, Player player, PlaceId trail[TRAIL_SIZE]);

/**
 * Return an array of `PlaceId`s giving all locations that the
 * current player could reach in their next move.
 *
 * The array can be in any order but must contain unique entries.
 * The array size is stored at the variable referenced by `n_locations`.
 * The current location should be included in the array.
 *
 * Road, rail, or sea connections should only be considered
 * if the `road`, `rail`, or `sea` parameters are true, respectively.
 */


PlaceId *HvGetDests (
 	HunterView hv, int *n_locations, bool road, bool rail, bool sea);

/**
 * Return an array of `PlaceId`s giving all of the locations that the
 * given `player` could reach from their current location.
 *
 * The array can be in any order but must contain unique entries.
 * The array size is stored at the variable referenced by `n_locations`.
 * The player's current location should be included in the array.
 *
 * `road`, `rail`, and `sea` connections should only be considered
 * if the `road`, `rail`, `sea` parameters are true, respectively.
 *
 * The function must take into account the current round for determining
 * how far `player` can travel by rail.
 *
 * If the player is Dracula, `n_locations` is set to 0, unless you know
 * Dracula's location precisely.
 */
 PlaceId *HvGetDestsPlayer (
 	HunterView hv, int *n_locations,
 	Player player, bool road, bool rail, bool sea);

#endif // !defined (FOD__HUNTER_VIEW_H_)
